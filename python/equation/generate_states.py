import sympy
import Orange
import orangol

def minus(st):
    """ if any(a,b,c, or d) are not zero, then you can substract
    those values. """
    new_states = []
    if st[0] != 0: # and st[0] != 1:
        new_states.append((0, st[1], sympy.simplify(st[2]-st[0]), st[3]))
    if st[1] != 0:
        new_states.append((st[0], 0, st[2], sympy.simplify(st[3]-st[1])))
    if st[2] != 0: # and st[2] != 1:
        new_states.append((sympy.simplify(st[0]-st[2]), st[1], 0, st[3]))
    if st[3] != 0:
        new_states.append((st[0], sympy.simplify(st[1]-st[3]), st[2], 0))
    return new_states

def divide(st):
    """ WHether a or c are not zero or 1, you can divide those values. """
    new_states = []
    if st[0] != 0 and st[0] != 1:
        new_states.append((1, sympy.simplify(st[1]/st[0]), sympy.simplify(st[2]/st[0]), sympy.simplify(st[3]/st[0])))
    if st[2] != 0 and st[2] != 1:
        new_states.append((sympy.simplify(st[0]/st[2]), sympy.simplify(st[1]/st[2]), 1, sympy.simplify(st[3]/st[2])))
    return new_states        
        

a = sympy.Symbol("a")
b = sympy.Symbol("b")
c = sympy.Symbol("c")
d = sympy.Symbol("d")

start = (a,b,c,d)
states = {}

toexplore = [start]
for st in toexplore:
    if st in states:
        continue
    states[st] = []
    mn = minus(st)
    dv = divide(st)
    states[st].extend(mn)
    states[st].extend(dv)
    toexplore.extend(states[st])
    
strstates = {}
for s in states:
    strstates[str(s)] = s
    
def a(self):
    return val_at(self, 0)

def b(self):
    return val_at(self, 1)

def c(self):
    return val_at(self, 2)

def d(self):
    return val_at(self, 3)

def val_at(self, i):
    val = strstates[self.state]
    if val[i] == 0:
        return "0"
    elif val[i] == 1:
        return "1"
    else:
        return "2"

def finished(self):
    val = strstates[self.state]
    if val[0] == 1 and val[1] == 0 and val[2] == 0:
        return "yes"
    return "no"    

class Equations(orangol.GOL_State):
    def __init__(self, skip_domain=False):
        self.state = str(start)
        if not skip_domain:
            self.create_domain()


    def create_domain(self):
        """ Prepares domain (attributes) used in GOL learning.
        """
        print "creating domain ..."
        a1 = Orange.feature.Discrete("a", values=["0","1","2"])
        a2 = Orange.feature.Discrete("b", values=["0","1","2"])
        a3 = Orange.feature.Discrete("c", values=["0","1","2"])
        a4 = Orange.feature.Discrete("d", values=["0","1","2"])
        a5 = Orange.feature.Discrete("Finished", values=["yes","no"])

        #self.attributes = [a2, a3, a4, a5, a11] + positions
        self.attributes = [a1, a2, a3, a4] + [a5]
        
        self.funcs = {a1.name: a, a2.name: b, a3.name: c,
                      a4.name: d, a5.name: finished}
            
        self.att_ids = {a.name:i for i,a in enumerate(self.attributes)}
        self.domain = Orange.data.Domain(self.attributes, a5)
        id = Orange.feature.String("id")
        mid = Orange.feature.Descriptor.new_meta_id()
        self.domain.add_meta(mid, id)

        eval = Orange.feature.Continuous("eval")    
        mid = Orange.feature.Descriptor.new_meta_id()
        self.domain.add_meta(mid, eval)    

    def create_example(self):
        example = Orange.data.Instance(self.domain)
        example["id"] = str(self.state)
        example["eval"] = self.evaluate()
        for a in self.attributes:
            example[a] = self.funcs[a.name](self)
        return example

    def get_all_states(self):
        all_states = []
        for k in strstates:
            self.state = k
            if finished(self) == "no":
                all_states.append(k)
        return all_states

    def id(self):
        return self.state

    def set_state(self, id):
        self.state = id

    def evaluate(self):
        return 1.0

    def get_moves(self):
        """ Method that generates possible moves. Basically you can just move the empty square. """
        moves = orangol.GOL_MoveList()
        new_states = states[strstates[self.state]]
        for ns in new_states:
            moves.append(EqMove(self.state, str(ns)))
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
        #print self.state
        #print self.attributes[at], self.attributes[at].name, self.funcs[self.attributes[at].name], self.funcs[self.attributes[at].name]()
        return Orange.data.Value(self.attributes[at], self.funcs[self.attributes[at].name](self))


        
    def deep_copy(self):
        n = Equations(skip_domain = True)
        n.state = self.state
        n.attributes = self.attributes
        a = n.attributes
#        n.funcs = {a[0].name: n.a, a[1].name: n.b, a[2].name: n.c,
#                      a[3].name: n.d, a[4].name: n.finished}        
        n.funcs = self.funcs
        n.att_ids = self.att_ids
        n.domain = self.domain
        return n
    

class EqMove(orangol.GOL_Move):
    def __new__(cls, old_state, new_state):
        self = super(EqMove, cls).__new__(cls)
        self.old_state = old_state
        self.new_state = new_state
        return self

    def __str__(self):
        return self.old_state + ":" + self.new_state    
