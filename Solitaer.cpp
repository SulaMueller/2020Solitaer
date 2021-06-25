
#include <cmath>
#include <string>

int lengthOfBoard = 7;
int lengthOfShortEdge = 3;
     
class Slot{
    // class to define a slot on the board, able to be occupied by a pin or not
public:
    int index;
    bool occupied;
    Slot();
    Slot(int, int);
    ~Slot(){};
    int neighboringIndex(std::str, int);
    void changeState();  // switch between occupied or not
};
Slot::Slot(int _index, int emptyslot){
    index = _index;
    occupied = index != emptyslot;
}
Slot::Slot(){
    index = -1;
    occupied = false;
}
int Slot::neighboringIndex(std::str dir, int shift){
    // get index of neighboring slot
    //@param dir:       string defining direction
    //@param shift:     size of step
    //@param return:    SlotIndex of neighboring slot, or -1 of no slot
    int i = (int)math.floor(index/lengthOfBoard);  // row
    int j = (int)math.floor(index % lengthOfBoard);  // column

    if (dir.compare("up")) i = i-shift;
    if (dir.compare("down")) i = i+shift;
    if (dir.compare("left")) j = j-shift;
    if (dir.compare("right")) j = j+shift;

    if (i >= 0 && j >= 0) 
        return i*lengthOfBoard + j;
    return -1;
}
void Slot::changeState(){
    occupied = !occupied;
}

class Board{
public:
    int squareboard[];
    Slot slots[];
    Board();
    ~Board();
    bool slotExists(int);
    int numSquare, numSlots;
private:
    int emptyslot;
    void initSquareboard();
    void clearCorners();
    void initSlots();
};
Board::Board(){
    numSquare = lengthOfBoard*lengthOfBoard;
    emptyslot = numSquare/2;  // only true for odd lengthOfBoard
    initSquareboard();
    initSlots();
}
Board::~Board(){
    delete[] squareboard;
    delete[] slots;
}
void Board::initSquareboard(){
    squareboard = new int[numSquare];
    for (int i=0; i<numSquare; i++) squareboard[i] = i;
    clearCorners();
}
void Board::clearCorners(){
    // corners have no slots since x-shaped board
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
        slots[i] = Slot(i, emptyslot);
}
bool Board::slotExists(int index){
    return squareboard[index] >= 0;
}

class Move{
    // class to define all moves that could in theory be done
    //@param referenceslot: left or upper slot in a row of three slots
    //@param dir:           either sideways (right)-> true or down -> false
public:
    bool dir, exists;
    int reference, middle, far;
    Move(Board&, int _reference, bool _dir);
    Move();
    ~Move(){};
    bool isPossible(Board& board);
    bool exactDir(Board& board);
    bool doMove(Board& board);
};
Move::Move(Board& board, int _reference, bool _dir){
    dir = _dir;
    reference = _reference;
    std::str dirstr = dir ? "right" : "down";
    middle = board.slots[reference].neighboringIndex(dirstr, 1);
    far = board.slots[reference].neighboringIndex(dirstr, 2);
    exists = board.slotExists(middle) && board.slotExists(far);
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
bool Move::exactDir(Board& board){
    return board.slots[reference].occupied;
}
bool Move::doMove(Board& board){
    if(!isPossible(board)) return false;
    board.slots[reference].changeState();
    board.slots[middle].changeState();
    board.slots[far].changeState();
    return true;
}

class Game{
public:
    Board board;
    Move moves[];
    Game();
    ~Game();
    int numMoves;
    int iterate(int numPins);
private:
    void initMoves();
    bool initSingleMove(int index, bool dir);
};
Game::Game(){
    board = Board();
    initMoves();
}
bool Game::initSingleMove(int index, bool dir){
    Move newmove = Move(board, index, dir);
    if (newmove.exists){
        moves[numMoves++] = newmove;
        return true;
    }
    return false;
}
void Game::initMoves(){
    int maxNumMoves = board.numSlots * 2;
    numMoves = 0;
    moves = new Move[maxNumMoves];
    for(int m=0; m<board.numSlots; m++){
        if(board.squareboard[m] >= 0){
            initSingleMove(m, false);
            initSingleMove(m, true);
        }
    }
}
Game::~Game(){
    delete[] moves;
}


Game g = Game();
