""" A script that computes goal-oriented strategy 
from user provided solution traces. 
"""

import Orange
import prolog.prolog_state as prolog_state
import gol
import meansends

# number of line edits between two states
SEARCH_DEPTH = 3
# number of unique traces covered by each strategy
MIN_TRACES = 1
# m-parameter
M = 20
MAX_BRANCHES = 1

# use "del" for delete and "member" for member 
state = prolog_state.Prolog("del")
data = gol.create_data_from_all_positions(state)

# learn prolog without implicit conditions
#learner = meansends.TheoryConceptualizer(
#    search_depth=SEARCH_DEPTH, active=False, m=M, att_alpha=0.1, 
#    min_traces=MIN_TRACES, min_seed_traces=MIN_TRACES, only_pure=False, 
#    only_positive=True, only_positive_implicit=True, max_conditions=5, 
#    min_quality=0.0, max_rules=5, add_implicit_conditions=False)

#plan = learner(data, state)
#print "Final plan"
#plan.print_plan()
# store plan
#plan.save_to_file(open("prolog_depth%d_member.txt"%SEARCH_DEPTH, "wt"))


# learn prolog with implicit conditions
learner = meansends.TheoryConceptualizer(
    search_depth=SEARCH_DEPTH, active=False, m=M, att_alpha=0.1, 
    min_traces=MIN_TRACES, min_seed_traces=MIN_TRACES, only_pure=False, 
    only_positive=True, only_positive_implicit=True, max_conditions=6, 
    min_quality=0.0, max_rules=1, add_implicit_conditions=True, 
    max_branches=MAX_BRANCHES, min_coverage = 20)
plan = learner(data, state)
print "Final plan with implicit conditions"
plan.print_plan()
# store plan
plan.save_to_file(open("prolog_depth%d_member_implicit.txt"%SEARCH_DEPTH, "wt"))
