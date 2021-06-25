import numpy as np

lengthOfBoard = 7
lengthOfShortEdge = 3

class SlotIndex:
    # class to give an index to every slot
    # @param index:     index (=name) of slot = index on a squared board
    # @param i,j:       row, column slot is in
    def __init__(self, index: int = None, i: int = None, j: int = None):
        self.index = index
        self.i = i
        self.j = j
        if index is not None and i is None or j is None:
            self.i = int(np.floor(index / (lengthOfBoard)))
            self.j = int(np.floor(index % (lengthOfBoard)))
        if index is None and i is not None and j is not None:
            self.index = i*(lengthOfBoard) + j
        
class Slot:
    # class to define a slot on the board, able to be occupied by a pin or not
    def __init__(self, board, indexname: int):
        self.index: SlotIndex
        self.occupied: bool

        self.index = SlotIndex(indexname)
        if self.index.index == board.emptySlot:
            self.occupied = False
        else:
            self.occupied = True
    
    def shiftIndex(self, dir: str, shift: int = 1) -> SlotIndex:
        # get index of neighboring slot
        # @param dir:       string defining direction
        # @param shift:     size of step
        # @param return:    SlotIndex of neighboring slot
        if dir is 'up':
            i = self.index.i - shift
            if i < 0: return None
            return SlotIndex(None, i, self.index.j)
        if dir is 'down':
            i = self.index.i + shift
            if i > lengthOfBoard-1: return None
            return SlotIndex(None, i, self.index.j)
        if dir is 'left':
            j = self.index.j - shift
            if j < 0: return None
            return SlotIndex(None, self.index.i, j)
        if dir is 'right':
            j = self.index.j + shift
            if j > lengthOfBoard-1: return None
            return SlotIndex(None, self.index.i, j)
    
    def changeState(self):
        self.occupied = not self.occupied

class ExistingMove:
    # class to define all moves that could in theory be done
    # @param referenceslot: left or upper slot in a row of three slots
    # @param dir:           either sideways (right) or down
    def __init__(self, board, referenceslot, dir: bool):
        self.dir = dir
        if dir:
            self.dirstr = 'down'
        else:
            self.dirstr = 'right'

        self.exists = False  # know from outside, if move exists
        farindex = referenceslot.shiftIndex(self.dirstr, 2)  # get third slot in a row
        if farindex is not None:  # everything else is only relevant if three slots are in a row
            if board.squareboard[farindex.i, farindex.j] > -1:  # and still on the board (not corner)
                self.exists = True
                self.referenceslot = referenceslot
                middleindex = referenceslot.shiftIndex(self.dirstr, 1)
                self.middleslot = board.slots[middleindex.index]
                self.farslot = board.slots[farindex.index]
    
    @property
    def isPossible(self) -> bool:
        if not self.middleslot.occupied: return False
        return self.referenceslot.occupied is not self.farslot.occupied

class MovesList:
    # class to store for each board-state, how many moves are possible and which one is made
    def __init__(self):
        self.numbersOfPossibleMoves = []
        self.usedMoveOption = []  # "index" of used move on list of possible moves
        self.usedMoves = []  # the actual move, saved in executed order (list of type ExistingMove)

    def appendState(self, numberOfPossibleMoves: int) -> bool:
        # append current board-state with number of possible moves
        if numberOfPossibleMoves == 0:
            return False
        self.numbersOfPossibleMoves.append(numberOfPossibleMoves)
        self.usedMoveOption.append(0)  # always chose first of possible options
        return True
    
    def increaseState(self, board) -> bool:
        # go to next possible move on list
        #print('INCREASING STATE')  # give some debug output
        #board.showBoard()
        board.doMove(None, undo=True)  # undo last move (which left no options)
        self.usedMoveOption[-1] = self.usedMoveOption[-1] + 1  # increase chosen option by one
        if self.numbersOfPossibleMoves[-1] == self.usedMoveOption[-1]:  # if list exhausted
            self.numbersOfPossibleMoves = self.numbersOfPossibleMoves[:-1]  # remove last element of lists
            self.usedMoveOption = self.usedMoveOption[:-1]
            return self.increaseState(board)  # increase option before
        return True
    
    def saveMove(self, move: ExistingMove = None):
        # saves move, if move is given and unsaves otherwise
        if move is not None:
            self.usedMoves.append(move)
        else:
            if len(self.usedMoves) > 0:
                self.usedMoves = self.usedMoves[:-1]  # delete last entry

class Board:
    # main class; stores current state of board and executes main commands
    def __init__(self):
        numEls = np.square(lengthOfBoard)

        # get center slot, staying empty at initialization
        self.emptySlot = int(np.floor(numEls/2))  # only correct for odd LengthOfBoard
        
        # get squared array describing indice of slots
        self.squareboard = np.linspace(0, numEls-1, numEls)
        self.squareboard = np.reshape(self.squareboard, [lengthOfBoard, -1])
        self.clearCorners()  # x-shape leaves corners free
        print('INDICE:')
        print(self.squareboard)

        # initialize instances
        self.getSlots()
        self.getExistingMoves()
        self.moveslist = MovesList()
        self.showBoard()

    def clearCorners(self):
        # corners have no slots since x-shaped board
        numDeletedElements = lengthOfBoard - lengthOfShortEdge
        deleteLeft = int(np.floor(numDeletedElements/2))
        deleteRight = int(numDeletedElements - deleteLeft)
        self.squareboard[0:deleteLeft, 0:deleteLeft] = -1
        self.squareboard[-deleteRight:lengthOfBoard, 0:deleteLeft] = -1
        self.squareboard[0:deleteLeft, -deleteRight:lengthOfBoard] = -1
        self.squareboard[-deleteRight:lengthOfBoard, -deleteRight:lengthOfBoard] = -1

    def getSlots(self):
        # initialize slots
        self.slots = {}
        for i in range(0, lengthOfBoard):
            for j in range(0, lengthOfBoard):
                if self.squareboard[i,j] > 0:
                    index = self.squareboard[i,j]
                    self.slots[index] = Slot(self, index)

    def getExistingMoves(self):
        # get a list of all moves that could be done in theory
        # cannot implement in getSlots because need neighboring slots to be initialized
        self.existingmoves = []
        for i in range(0, lengthOfBoard):
            for j in range(0, lengthOfBoard):
                if self.squareboard[i,j] > 0:
                    index = self.squareboard[i,j]
                    move_d = ExistingMove(self, self.slots[index], True)  # 3 vertical slots in row
                    move_r = ExistingMove(self, self.slots[index], False)  # 3 horizontal slots in row
                    if move_d.exists:
                        self.existingmoves.append(move_d)
                    if move_r.exists:
                        self.existingmoves.append(move_r)

    def showBoard(self):
        # visualize board
        showboard = np.ndarray([lengthOfBoard, lengthOfBoard])
        for i in range(0, lengthOfBoard):
            for j in range(0, lengthOfBoard):
                index = self.squareboard[i,j]
                if index == -1:
                    showboard[i,j] = 0
                elif self.slots.get(index).occupied:
                    showboard[i,j] = 88
                else:
                    showboard[i,j] = 1
        print(' ')
        print(showboard)

    def getNumberOfFreeSlots(self, free: bool = True) -> int:
        # count number of free or occupied slots
        # @param free:  define if free slots (True) or occupied slots are counted
        numberOfFreeSlots = 0
        for key in self.slots:
            if self.slots[key].occupied != free:
                numberOfFreeSlots = numberOfFreeSlots+1
        return numberOfFreeSlots

    def getNumberOfPossibleMoves(self, numberOfMoveToDo = 0) -> [int, ExistingMove]:
        # for specific board-state, count how many moves are theoretically possible
        # @param numberOfMoveToDo:  on list of possible moves, move option to be chosen
        # @return:                  [numberOfPossibleMoves, the move to be done]
        numberOfMoves = 0
        returnMove = None
        for move in self.existingmoves:
            if move.isPossible:
                if numberOfMoves == numberOfMoveToDo:
                    returnMove = move
                numberOfMoves = numberOfMoves + 1
        return [numberOfMoves, returnMove]

    def getMove(self) -> ExistingMove:
        # find out which move is to be done
        moveoption = self.moveslist.usedMoveOption[-1]  # the to choose option
        [_, returnMove] = self.getNumberOfPossibleMoves(moveoption)
        return returnMove

    def doMove(self, move = None, undo: bool = False):
        # do the move
        # @param undo:  if True, will undo last move
        if move is None and not undo:  # get next move option
            move = self.getMove()
        if move is None and undo:  # get last move that was done
            move = self.moveslist.usedMoves[-1]
        move.referenceslot.changeState()  # fill/ empty slots
        move.middleslot.changeState()
        move.farslot.changeState()
        if undo: move = None  # saveMove(None) will delete last entry
        self.moveslist.saveMove(move)  # save move on list

    def nextMove(self):
        # wrapper function for everything that needs to be done for a move
        [numberOfPossibleMoves, nextMove] = self.getNumberOfPossibleMoves()
        if not self.moveslist.appendState(numberOfPossibleMoves):  # if no possible moves for current configuration
            self.moveslist.increaseState(self)  # goes to next working option and undoes last moves
            nextMove = self.getMove()
        self.doMove(nextMove)
        #self.showBoard()
    
    def disrupt(self) -> bool:
        # if very first move is changed -> results will be symmetrical
        if len(self.moveslist.usedMoveOption) < 1: return False
        return self.moveslist.usedMoveOption[0] > 0 
    
    def findMoveCombination(self):
        # main function to find a combination of moves
        while self.getNumberOfFreeSlots(free=False) > 1 and not self.disrupt():
            self.nextMove()
        self.showAllMoves()

    def showAllMoves(self):
        # after combination is found, show it using a brandnew board
        b = Board()
        for move in self.moveslist.usedMoves:
            b.doMove(move)
            b.showBoard()

board = Board()
board.findMoveCombination()
