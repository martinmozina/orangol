import random
import Orange
import collections
import orangol

neighbours = {
    0: [1,3],
    1: [0,2,4],
    2: [1,5],
    3: [0,4,6],
    4: [1,3,5,7],
    5: [2,4,8],
    6: [3,7],
    7: [4,6,8],
    8: [5,7]}

def optimal(self):
    return self.evaluate()

def finished(self):
    return "yes" if self.state == "123456780" else "no"

def manhattan(self):
    opt = "123456780"
    man = 0
    for i,o in enumerate(opt):
        if o == "0":
            continue
        xo, yo = i/3, i%3
        indx = self.state.index(o)
        xs, ys = indx/3, indx%3
        man += abs(xo-xs)+abs(yo-ys)
    return man

def manhattan_prod(self):
    opt = "123456780"
    man = 1
    for i,o in enumerate(opt):
        if o == "0":
            continue
        xo, yo = i/3, i%3
        indx = self.state.index(o)
        xs, ys = indx/3, indx%3
        man *= 1 + abs(xo-xs)+abs(yo-ys)
    return man

def max_manh(self):
    """ sum of manhattan, powered, where base is 10. """
    opt = "123456780"
    man = 0
    for i,o in enumerate(opt):
        if o == "0":
            continue
        xo, yo = i/3, i%3
        indx = self.state.index(o)
        xs, ys = indx/3, indx%3
#        man = max(man, abs(xo-xs)+abs(yo-ys))
        man += 10 ** (abs(xo-xs)+abs(yo-ys))
    return man
    

def ncorrect(self):
    opt = "123456780"
    corr = 0
    for i,o in enumerate(opt):
        if self.state[i] == o:
            corr += 1
    return corr

def ncorrect_ordered(self):
    opt = "123456780"
    corr = 0
    for i,o in enumerate(opt):
        if self.state[i] == o:
            corr += 1
        else:
            break
    return corr
    
def manhattan_first(self):
    opt = "123456780"
    first = None
    for i,o in enumerate(opt):
        if self.state[i] != o:
            first = o
            break

    if not first:
        return 0
    xo, yo = opt.index(first)/3, opt.index(first)%3
    indx = self.state.index(first)
    xs, ys = indx/3, indx%3
    return abs(xo-xs)+abs(yo-ys)+4*(len(opt)-opt.index(first)) 

def manhattan_first_zero(self):
    opt = "123456780"
    first = None
    for i,o in enumerate(opt):
        if self.state[i] != o:
            first = o
            break

    if not first:
        return 0
    xo, yo = opt.index(first)/3, opt.index(first)%3
    indx = self.state.index(first)
    xs, ys = indx/3, indx%3
    ind0 = self.state.index("0")
    xs0, ys0 = ind0/3, ind0%3
    return abs(xo-xs)+abs(yo-ys)+4*(len(opt)-opt.index(first))+\
           abs(xo-xs0)+abs(yo-ys0)

def distance_zero_first(self):
    opt = "123456780"
    first = None
    for i,o in enumerate(opt):
        if self.state[i] != o:
            first = o
            break

    if not first:
        return 0
    xo, yo = opt.index(first)/3, opt.index(first)%3
    ind0 = self.state.index("0")
    xs0, ys0 = ind0/3, ind0%3
    return abs(xo-xs0)+abs(yo-ys0)

def distance_zero_mid(self):
    ind0 = self.state.index("0")
    xs0, ys0 = ind0/3, ind0%3
    return abs(1-xs0)+abs(1-ys0)

def distance_pairs(self):
    # x,y of all values
    coord = {}
    for i in range(9):
        coord[i] = (self.state.index(str(i))/3, self.state.index(str(i))%3)
    
    pairs = [(1,2),(1,4),(2,3),(2,5),(3,6),(4,5),(4,7),(5,6),(5,8),(6,0),(7,8),(8,0)]
    dist = 0
    for p1, p2 in pairs:
        x1,y1 = coord[p1]
        x2,y2 = coord[p2]
        dist += abs(x1-x2)+abs(y1-y2)-1
    return dist

def sequence_score(self):
    inv_state = self.state[:3]+self.state[5:2:-1]+self.state[6:]
    return (manhattan(self) + 3*(inv_state[-1] != '0') + 
            3*sum(int(inv_state[i-1]) != int(inv_state[i]) + 1 
            for i in range(9) if inv_state[i-1] != '0'))

def pos123(self):
    return ("yes" if self.state[3] == '1' and 
                     self.state[0] == '2' and 
                     self.state[1] == '3' 
            else "no")
 
def pos741(self):
    return ("yes" if self.state[3] == '7' and 
                     self.state[0] == '4' and 
                     self.state[1] == '1' 
            else "no")

class Position(object):
    def __init__(self, number, position):
        self.number = str(number)
        self.xo, self.yo = position/3, position%3
        
    def __call__(self, state):
        indx = state.state.index(self.number)
        xs, ys = indx/3, indx%3
        return abs(self.xo-xs)+abs(self.yo-ys)
    
class PositionDiscrete(object):
    def __init__(self, number, position):
        self.number = str(number)
        self.xo, self.yo = position/3, position%3
        
    def __call__(self, state):
        indx = state.state.index(self.number)
        xs, ys = indx/3, indx%3
        return "yes" if xs == self.xo and ys == self.yo else "no"    

class Puzzle8(orangol.GOL_State):
    def __init__(self, skip_domain=False):
        self.state = "012345678"
        self.returnDTG = True
        if not skip_domain:
            self.create_domain()

    def load_table(self):
        self.dtg = {}
        for l in open("D:\\work\\domains\\chess\\kbnk\\goalol\\puzzle\\db3x3.txt"):
            desc, nmoves = l.strip().split(",")
            self.dtg[desc] = float(nmoves)
            if self.dtg[desc] < 0:
                self.dtg[desc] = -self.dtg[desc]
        print "dtg_len:", len(self.dtg)
        
    def set_traces(self, traces):
        self.example_traces = collections.defaultdict(set)
        for it, trace in enumerate(traces):
            for tr in trace:
                self.example_traces[tr].add(str(it))

    def create_domain(self):
        """ Prepares domain (attributes) used in GOL learning.
        """
        print "creating domain ..."
        a1 = Orange.feature.Continuous("Optimal")
        a2 = Orange.feature.Continuous("Manhattan")
        a3 = Orange.feature.Continuous("NCorrect")
        a4 = Orange.feature.Continuous("NCorrectOrdered")
        a5 = Orange.feature.Continuous("ManhattanFirst")
        a6 = Orange.feature.Continuous("ManhattanFirstZero")
        a7 = Orange.feature.Continuous("ManhattanProd")
        a8 = Orange.feature.Continuous("DistanceZeroMid")
        a9 = Orange.feature.Continuous("DistanceZeroFirst")
        a10 = Orange.feature.Discrete("Finished", values=["yes","no"])
        a11 = Orange.feature.Continuous("MaxMDistance")
        a12 = Orange.feature.Continuous("DistPairs")
        a13 = Orange.feature.Continuous("SequenceScore")
        a14 = Orange.feature.Discrete("123on401", values=["yes","no"])
        a15 = Orange.feature.Discrete("741on401", values=["yes","no"])
        #positions = [Orange.feature.Continuous("Dist(%d on place%d)"%(i,v)) for i in range(9) for v in range(9)]
        positions = [Orange.feature.Discrete("Dist(%d on place%d)"%(i,v), values = ["yes", "no"]) 
                     for i in range(9) for v in range(9)]
        
        #self.attributes = [a2, a3, a4, a5, a11] + positions
        self.attributes = positions + [a10]
        
        

        self.funcs = {a1.name: optimal, a2.name: manhattan, a3.name: ncorrect,
                      a4.name: ncorrect_ordered, a5.name: manhattan_first,
                      a6.name: manhattan_first_zero, a7.name: manhattan_prod,
                      a8.name: distance_zero_mid, a9.name: distance_zero_first,
                      a10.name: finished, a11.name: max_manh, a12.name: distance_pairs,
                      a13.name: sequence_score, a14.name: pos123, a15.name: pos741}
        for pi in range(9):
            for vi in range(9):
                #self.funcs["Dist(%d on place%d)"%(pi,vi)] = Position(pi,vi)
                self.funcs["Dist(%d on place%d)"%(pi,vi)] = PositionDiscrete(pi,vi)
                
#        for ai, a in enumerate(self.attributes):
#            print "Index: %d, name = %s"%(ai, a.name)
            
        self.att_ids = {a.name:i for i,a in enumerate(self.attributes)}
        self.domain = Orange.data.Domain(self.attributes, a10)
        id = Orange.feature.String("id")
        mid = Orange.feature.Descriptor.new_meta_id()
        self.domain.add_meta(mid, id)

        eval = Orange.feature.Continuous("eval")    
        mid = Orange.feature.Descriptor.new_meta_id()
        self.domain.add_meta(mid, eval)
        
        trace = Orange.feature.String("trace")    
        mid = Orange.feature.Descriptor.new_meta_id()
        self.domain.add_meta(mid, trace)
        
    def create_example(self):
        example = Orange.data.Instance(self.domain)
        example["id"] = self.state
        example["eval"] = self.evaluate()
        if self.state in self.example_traces:
            example["trace"] = ";".join(self.example_traces[self.state])
        else:
            example["trace"] = ""
        for a in self.attributes:
            example[a] = self.funcs[a.name](self)
        return example

    def id(self):
        return self.state

    def set_state(self, id):
        self.state = id
        
    def set_random_state(self):
        while True:
            self.state = random.choice(self.dtg.keys())
            v = self.evaluate()
            if v < 100 and v > 0:
                return

    def evaluate(self):
        if self.returnDTG:
            return self.dtg[self.state]
        if self.dtg[self.state] == 0:
            return 0.
        if self.dtg[self.state] > 50:
            return self.dtg[self.state]
        return self.dtg[self.state]

    def get_moves(self):
        """ Method that generates possible moves. 
        Basically it is moving the empty square. 
        """
        moves = orangol.GOL_MoveList()
        ind0 = self.state.index("0")
        for indx in neighbours[ind0]:
            moves.append(PuzzMove(self.state, ind0, indx))
        return moves
                               
    def or_node(self):
        return True

    def do_move(self, move):
        self.state = move.new_state
            
    def undo_move(self, move):
        self.state = move.old_state

    def prune_prob(self, state, goal_list, max_depth, depth):
        return 0.

    def get_attribute_value(self, at):
        """ Returns value of at in this current state.
            It has to be of Orange.data.Value type. """
        return Orange.data.Value(self.attributes[at], 
                                 self.funcs[self.attributes[at].name](self))
        
    def deep_copy(self):
        n = Puzzle8(skip_domain = True)
        n.state = self.state
        n.dtg  = self.dtg
        n.attributes = self.attributes
        n.funcs = self.funcs
        n.att_ids = self.att_ids
        n.domain = self.domain
        n.returnDTG = self.returnDTG
        return n

class PuzzMove(orangol.GOL_Move):
    def __new__(cls, state, old_index, new_index):
        self = super(PuzzMove, cls).__new__(cls)
        self.old_state = state
        self.new_state = list(state)
        self.new_state[new_index] = "0"
        self.new_state[old_index] = state[new_index]
        self.new_state = "".join(self.new_state)
        return self

    def __str__(self):
        return self.old_state + ":" + self.new_state

#===============================================================================
# if __name__ == "__main__":
#     s1 = Puzzle8()
#     s1.load_table()
#     states = ["403657812", "456207813", "478265310", "506471382", 
#               "528630714", "568240371", "612078354", "642187035", "642310857"]
#     sts = [s1]
#     for i in range(9):
#         si = s1.deep_copy()
#         si.state = states[i]
#         sts.append(si)
#     import assignGoals
#     assignGoals.fromStartPosition(sts, "test6320_puzzle.tab", 6, 3, 20)
#===============================================================================

        



