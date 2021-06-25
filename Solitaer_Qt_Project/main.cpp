
#include <cmath>
#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>

using namespace std;

int lengthOfBoard = 7;
int lengthOfShortEdge = 3;
int numLeftPins = 1;  // indicate, how many pins should be left at the end

bool DEBUG = false;
int linelen = 50;

class Slot{
    // class to define a slot on the board, able to be occupied by a pin or not
public:
    int index;  // "name" of slot (not strictly ascending, since some positions don't have a slot)
    bool occupied;  // by a pin
    Slot(int, int);
    Slot();
    ~Slot(){};
    int neighboringIndex(string dir, int shift);  // give index of neighboring slot
    void changeState();  // switch between occupied or not
};
Slot::Slot(int _index, int emptyslot){
    //@param emptyslot: index of slot that is left empty at init of board
    index = _index;
    occupied = (index != emptyslot) && (index >= 0);
}
Slot::Slot(){  // need default constructor
    index = -1;
    occupied = false;
}
int Slot::neighboringIndex(string dir, int shift){
    // get index of neighboring slot
    //@param dir:       string defining direction (up, down, left, right)
    //@param shift:     size of step
    //@param return:    SlotIndex of neighboring slot, or -1 if no slot
    int i = (int)floor(index / lengthOfBoard);  // row
    int j = (int)floor(index % lengthOfBoard);  // column

    if (dir.compare("up")==0) i = i-shift;
    if (dir.compare("down")==0) i = i+shift;
    if (dir.compare("left")==0) j = j-shift;
    if (dir.compare("right")==0) j = j+shift;

    if (i >= 0 && j >= 0)
        return i*lengthOfBoard + j;
    return -1;
}
void Slot::changeState(){
    if (index > -1)
        occupied = !occupied;
}

class Board{
    // class to define a board with slots
public:
    int numSquare,  // lengthOfBoard^2 (includes empty corners)
        numSlots,   // number of live slots (constant, excludes empty corners)
        numPins;    // number of occupied slots (variable)
    int* squareboard;  // indice of all slots [numSquare]; -1 if there is no slot
    int* edges;  // list of all indice on (short) edges [4*lengthOfShortEdge]
    Slot* slots;  // array of all slots [numSquare]
    Board();
    ~Board();
    bool slotExists(int);  // check if there is a slot on this position of the squareboard
    void plotBoard();  // debug output
    void printSquareboard();  // debug output
private:
    int emptyslot;  // initial position of empty slot
    void initSquareboard();
    void clearCorners();  // clear corners of squareboard (where there are no slots)
    void initSlots();
    void initEdges();
};
Board::Board(){
    if (DEBUG) cout << "\ninitializing Board...";
    numSquare = lengthOfBoard*lengthOfBoard;
    emptyslot = numSquare/2;  // only true for odd lengthOfBoard
    initSquareboard();
    initSlots();
    initEdges();
    if (DEBUG) plotBoard();
}
void Board::initSquareboard(){
    squareboard = new int[numSquare];
    for (int i=0; i<numSquare; i++) squareboard[i] = i;
    clearCorners();
    if (DEBUG) printSquareboard();
}
void Board::clearCorners(){
    // corners have no slots since x-shaped board
    // -> set all entries of squareboard to -1 if corner
    int numDeletedElements = lengthOfBoard - lengthOfShortEdge;
    int deleteLeft = numDeletedElements/2;  // automatic floor
    int deleteRight = lengthOfBoard - (numDeletedElements - deleteLeft) -1;
    numSlots = numSquare;
    for (int i=0; i<lengthOfBoard; i++){
        for (int j=0; j<lengthOfBoard; j++){
            bool cond = false;
            cond += (i<deleteLeft && j<deleteLeft);
            cond += (i<deleteLeft && j>deleteRight);
            cond += (i>deleteRight && j<deleteLeft);
            cond += (i>deleteRight && j>deleteRight);
            if(cond){
                squareboard[i*lengthOfBoard+j] = -1;
                numSlots--;
            }
        }
    }
}
void Board::initSlots(){
    // initialize slots
    slots = new Slot[numSquare];
    for(int i=0; i<numSquare; i++)
        slots[i] = Slot(squareboard[i], emptyslot);
    numPins = numSlots-1;  // since one slot is empty on init
}
void Board::initEdges(){
    // need edges for sanity check later (detectProblem)
    edges = new int[4*lengthOfShortEdge];
    int edgeind = 0;
    for(int j=0; j<lengthOfBoard; j++)  // top edge
        if(squareboard[j]>=0)
            edges[edgeind++] = j;
    for(int j=0; j<lengthOfBoard; j++)  // bottom edge
        if (squareboard[(lengthOfBoard-1)*lengthOfBoard + j]>=0)
            edges[edgeind++] = (lengthOfBoard-1)*lengthOfBoard + j;
    for(int i=0; i<lengthOfBoard; i++)  // left edge
        if (squareboard[i*lengthOfBoard]>=0)
            edges[edgeind++] = i*lengthOfBoard;
    for(int i=1; i<lengthOfBoard+1; i++)  // right edge
        if (squareboard[i*lengthOfBoard-1]>=0)
            edges[edgeind++] = i*lengthOfBoard-1;
}
bool Board::slotExists(int index){
    if (index < 0) return false;
    return squareboard[index] >= 0;
}
void Board::printSquareboard(){
    cout << "\n\nSQUAREBOARD:\n";
    for (int i=0; i<numSquare; i++){
        if (squareboard[i] == -1) cout << "   ";
        else{
            if(squareboard[i] < 10) cout << " ";
            cout << " " << squareboard[i];
        }
        if ((i+1)%lengthOfBoard == 0) cout << "\n";
    }
}
void Board::plotBoard(){
    cout << "\n\nCURRENT BOARD:\n";
    for (int i=0; i<numSquare; i++){
        cout << " ";
        if (squareboard[i] < 0) cout << " ";
        else{
            if (slots[squareboard[i]].occupied) cout << "O";
            else cout << "-";
        }
        if ((i+1)%lengthOfBoard == 0) cout << "\n";
    }
}
Board::~Board(){
    //delete[] squareboard;
    //delete[] slots;
    //delete[] edges;
}

class Move{
    // class to define all moves that could in theory be done
    // a move involves three slots in a row [reference, middle, far]
    // row can be horizontal (dir = true) or vertical (dir = false)
    //@param referenceslot: left or upper slot in a row of three slots
    //@param dir:           horizontal -> : true / vertical v : false
public:
    bool dir,  // horizontal or vertical
         exists;  // move can be done IN PRINCIPLE -> all slots are on the board (constant)
    int reference, middle, far;  // indice of involved slots
    Move(Board&, int _reference, bool _dir);
    Move();
    ~Move(){};
    bool doMove(Board& board);  // change slots on board according to move
    bool undoMove(Board& board);  // change slots on board like they were before a move was done
    bool isPossible(Board& board);  // move can be done AT THE MOMENT (depends on current state of board)
    bool isPossibleUndo(Board& board);
    void plotMove(Board& board);
};
Move::Move(Board& board, int _reference, bool _dir){
    dir = _dir;
    reference = _reference;
    string dirstr = dir ? "right" : "down";
    middle = board.slots[reference].neighboringIndex(dirstr, 1);
    far = board.slots[reference].neighboringIndex(dirstr, 2);
    exists = board.slotExists(middle) && board.slotExists(far);
    if (dir) exists = exists && (reference/lengthOfBoard == far/lengthOfBoard);  // exclude line wrapping
}
Move::Move(){
    dir = false;
    reference = -1;
    middle = -1;
    far = -1;
    exists = false;
}
bool Move::isPossible(Board& board){
    if (!exists) return false;
    return board.slots[middle].occupied && (board.slots[reference].occupied != board.slots[far].occupied);
}
bool Move::doMove(Board& board){
    if (!isPossible(board)) return false;
    board.slots[reference].changeState();
    board.slots[middle].changeState();
    board.slots[far].changeState();
    return true;
}
bool Move::isPossibleUndo(Board& board){
    if (!exists) return false;
    return !board.slots[middle].occupied && (board.slots[reference].occupied != board.slots[far].occupied);
}
bool Move::undoMove(Board& board){
    if (!isPossibleUndo(board)) return false;
    board.slots[reference].changeState();
    board.slots[middle].changeState();
    board.slots[far].changeState();
    return true;
}
void printSpace(int n){
    for(int i=0; i<n; i++) cout << " ";
}
void Move::plotMove(Board &board){
    cout << "\n \nEXECUTED MOVE: ";
    printSpace(3*lengthOfBoard-15);
    cout<< "RESULTING BOARD:\n";
    for (int i=0; i<lengthOfBoard; i++){
        // move
        for (int j=0; j<lengthOfBoard; j++){
            cout << " ";
            int slotind = i*lengthOfBoard+j;
            if (board.squareboard[slotind] < 0) cout << " ";
            else{
                if ((slotind==reference) || (slotind==middle) || (slotind==far)){
                    if (board.slots[board.squareboard[slotind]].occupied) cout << "x";
                    else cout << "X";
                }
                else{
                    if (board.slots[board.squareboard[slotind]].occupied) cout << "O";
                    else cout << "-";
                }
            }
        }
        // space
        printSpace(lengthOfBoard);
        // resulting board
        for (int j=0; j<lengthOfBoard; j++){
            cout << " ";
            int slotind = i*lengthOfBoard+j;
            if (board.squareboard[slotind] < 0) cout << " ";
            else{
                if (board.slots[board.squareboard[slotind]].occupied) cout << "O";
                else cout << "-";
            }
        }
        cout << "\n";
    }
}

class Game{
    // class to start a game, iterate through possible moves etc
    // main class
public:
    int numIts,  // count failed attempts
        minNumPins;  // remember how good best so far solution was
    time_t start, finish;  // measure execution time
    Board board;
    Game();
    ~Game();
    bool iterate(int numPins);
    void print(string);
    void plotAllMoves();
private:
    int numExistingMoves,  // number of moves that are theoretically possible ("existing moves")
        numSavedMoves;  // number of executed moves
    Move* existingMoves;  // array of all moves that are theoretically possible [numExistingMoves]
    int* savedMoves;  // all executed moves in correct order [numSlots -2]
    int* possibleMoves;  // all possible moves for each executed move [numExistingMoves*(numSlots-2)]
    int* numPossibleMoves;  // for each executed move, save how many moves are possible (max of possibleMoves)
    int* executedMovePtrs;  // for each executed move, save which move was done (ptr on list of possibleMoves)

    bool initSingleMove(int index, bool dir);
    void initExistingMoves();  // init array of all existing moves
    void initMoveLists();  // init all above move lists

    bool doMove(int moveind);  // do a specified move ("moveind" as index on existingMoves)
    bool undoMoves(int numMoves);  // undo last numMoves moves
    int getCurMoveslist();  // get list of all moves that are currently possible
    int getMoveindFromPossibleMoves(int moveOnList);  // get "moveind" from possibleMoves

    bool detectProblem();  // do sanity check
    bool reactToProblem();  // undo a few moves if check fails
    bool checkIfProblem();  // summarize both
    bool resolveDeadEnd();  // undo a few moves if there are no options left

    bool initIteration();
    bool nextMove();
    bool incCurMove();  // do next move on list of possible moves for current move

    void printHeader();
    void printState();
    void printPlotExplanation();
};
Game::Game(){
    printHeader();
    if (DEBUG) print("\ninitializing Game...\n");
    board = Board();
    if (DEBUG) print("\ninitializing Moves...\n");
    initExistingMoves();
    initMoveLists();
    numIts = 0;
    numSavedMoves = 0;
    minNumPins = board.numPins;
}
bool Game::initSingleMove(int index, bool dir){
    // check if a move exists and add to existingMoves if it does
    Move newmove = Move(board, index, dir);
    if (newmove.exists){
        existingMoves[numExistingMoves++] = newmove;
        return true;
    }
    return false;
}
void Game::initExistingMoves(){
    // initialize all moves that could theoretically come up at some board-state
    int maxNumMoves = board.numSlots * 2;  // theoretical maximum
    numExistingMoves = 0;  // count how many they actually are
    existingMoves = new Move[maxNumMoves];
    for(int m=0; m<board.numSquare; m++){
        if(board.squareboard[m] >= 0){
            initSingleMove(m, false);
            initSingleMove(m, true);
        }
    }
    if (DEBUG) cout << "\nNumber of existing moves: " << numExistingMoves;
}
void Game::initMoveLists(){
    savedMoves = new int[board.numSlots-2];  // -1 for empty slot, -1 for remaining pin
    executedMovePtrs = new int[board.numSlots-2];
    memset(executedMovePtrs, 0, (board.numSlots-2)*sizeof(int));
    numPossibleMoves = new int[board.numSlots-2];
    possibleMoves = new int[(board.numSlots-2) * numExistingMoves];
    memset(possibleMoves, -1, (board.numSlots-2) * numExistingMoves*sizeof(int));
}
Game::~Game(){
    delete[] existingMoves;
    delete[] savedMoves;
    delete [] possibleMoves;
    delete [] numPossibleMoves;
    delete [] executedMovePtrs;
}
bool Game::doMove(int moveind){
    //@param moveind: index of move in existingMoves
    Move currmove = existingMoves[moveind];
    if(!currmove.doMove(board)) return false;  // change pins
    savedMoves[numSavedMoves++] = moveind;
    board.numPins--;
    if(DEBUG) currmove.plotMove(board);
    return true;
}
bool Game::undoMoves(int numMoves){
    for(int m=0; m<numMoves; m++){
        executedMovePtrs[numSavedMoves--] = 0;  // restart at first possible move for subsequent move
        int moveind = savedMoves[numSavedMoves];  // get index on existingMoves of executed move
        Move currmove = existingMoves[moveind];
        if(!currmove.undoMove(board)) return false;
        if(DEBUG) print("undoing move");
        if(DEBUG) currmove.plotMove(board);
    }
    board.numPins += numMoves;
    return true;
}
int Game::getCurMoveslist(){
    // get list of all moves that are possible at moment of call
    // add all possible moves to possibleMoves at column for current move
    //@return: number of possible moves
    int numPossMoves = 0;
    for(int m=0; m<numExistingMoves; m++){
        if(existingMoves[m].isPossible(board)){
            possibleMoves[numSavedMoves*numExistingMoves+numPossMoves] = m;
            numPossMoves++;
        }
    }
    numPossibleMoves[numSavedMoves] = numPossMoves;
    executedMovePtrs[numSavedMoves] = 0;  // always start at first posMove in list (gCMl isn't called again for identical board)
    return numPossMoves;
}
int Game::getMoveindFromPossibleMoves(int moveOnList){
    // get index on existingMoves from chosen possibility on possibleMoves
    return possibleMoves[numSavedMoves*numExistingMoves+moveOnList];
}
bool Game::detectProblem(){
    // rough check if board is still solvable
    // method: check how many "legs" (= edges) are still to be freed
    //          -> need at least three pins in the middle for each leg to free them
    int numLostLegs = 0;
    int numFreePins = board.numPins;
    for(int leg=0; leg<4; leg++){
        bool legLost = false;
        for(int pin=0; pin<lengthOfShortEdge; pin++){
            int slotind = board.edges[leg*lengthOfShortEdge+pin];
            if (board.slots[slotind].occupied){
                legLost = true;
                numFreePins--;
            }
        }
        if (legLost) numLostLegs++;
    }
    return (numFreePins < numLostLegs*3) && (numLostLegs>1);
}
bool Game::reactToProblem(){
    undoMoves(1);
    while(detectProblem()) undoMoves(1);
    return incCurMove();
}
bool Game::checkIfProblem(){
    // summarize detect and solve problem
    if (board.numPins<15)
        if(detectProblem()) {
            if (DEBUG) print("Problem detected");
            return reactToProblem();
        }
    return true;
}
bool Game::resolveDeadEnd(){
    if (board.numPins < minNumPins)  // save how good it became
        minNumPins = board.numPins;
    undoMoves(board.numPins/2);
    numIts++;
    if(numIts%10000 == 0) printState();
    return incCurMove();
}
bool Game::initIteration(){
    getCurMoveslist();
    doMove(getMoveindFromPossibleMoves(0));  // do first possible move
    return true;
}
bool Game::nextMove(){
    // just do next move
    if (getCurMoveslist()==0) return resolveDeadEnd();
    doMove(getMoveindFromPossibleMoves(0));  // do first possible move
    return checkIfProblem();
}
bool Game::incCurMove(){
    // if a move ran into dead end or problem
    //      -> go back to sane stage by undoing
    //      -> do a move that wasn't done before (next on list of possible moves for last undone move)
    if(numSavedMoves<=1) return false;  // if very first move fails -> everything fails
    executedMovePtrs[numSavedMoves]++;  // go to next possible move
    if (executedMovePtrs[numSavedMoves] >= numPossibleMoves[numSavedMoves]){
        // if list of possible moves is exhausted
        undoMoves(1);
        return incCurMove();  // carry-over and inc previous move
    }
    doMove(getMoveindFromPossibleMoves(executedMovePtrs[numSavedMoves]));  // do next possile move
    return nextMove();
}
bool Game::iterate(int numPins){
    // main function to find solution
    //@param numPins: number of pins left at the end
    time(&start);
    initIteration();
    while(board.numPins > numPins)
        if(!nextMove()) return false;
    time(&finish);
    return true;
}
void Game::print(string s){
    cout << "\n" << s;
}
void printThickLine(){
    cout << " \n ";
    for(int i=0; i<linelen-2; i++) cout <<"=";
}
void printInThickLines(string s){
    int space = linelen - s.length() - 4;
    int s1 = space/2;
    int s2 = space -s1;
    cout <<"\n||";
    printSpace(s1);
    cout << s;
    printSpace(s2);
    cout << "||";
}
void Game::printHeader(){
    printThickLine();
    printInThickLines(" ");
    printInThickLines("Script to solve the game \"Solitaer\"");
    printInThickLines("by Sula Mueller (12/2020)");
    printInThickLines(" ");
    printThickLine();
    print(" \n");
}
void Game::printState(){
    cout << "\nIteration: " << numIts << "; best result: " << minNumPins;
}
void Game::printPlotExplanation(){
    print(" \n ");
    printThickLine();
    printInThickLines(" ");
    printInThickLines("PLOTTING RESULTING MOVES");
    printInThickLines(" ");
    printThickLine();
    print("\n \n  O : slot with pin \n  - : slot without pin \n  X : pins that jump over each other \n  x : previous free slot a pin jumps into");
    print(" \n ");
    print("\n LEFT: move to be done     RIGHT: board after move\n");
}
void Game::plotAllMoves(){
    printPlotExplanation();
    Board showboard = Board();
    for(int m=0; m<numSavedMoves; m++){
        Move currmov = existingMoves[savedMoves[m]];
        currmov.doMove(showboard);
        currmov.plotMove(showboard);
    }
    cout << "\n \nNUMBER OF ITERATIONS = " << numIts;
    cout << "\nPROCESSING TIME = " << finish-start << "s";
}


int main(){
    Game g = Game();
    g.iterate(numLeftPins);  // find a solution for (n) number of pins left
    g.plotAllMoves();
    g.print("\nDone.\n \n");
}

