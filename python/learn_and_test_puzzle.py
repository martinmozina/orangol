import Orange
#import puzzle_gol_all_attributes as puzzle_gol
import puzzle.puzzle_gol as puzzle_gol
import gol
import meansends
import random
random.seed(42)
import evaluate

# PREPARING LEARNING EXAMPLES
N = 50
SEARCH_DEPTH = 5
MIN_TRACES = 1
COV_WEIGHT = 5
M = 20
MAX_BRANCHES = 10

state = puzzle_gol.Puzzle8()
state.load_table()
state.set_random_state()
posits = []
for i in range(N):
    state_tmp = state.deep_copy()
    state_tmp.set_random_state()
    posits.append(state_tmp)
data, traces = gol.create_data_from_start_positions(posits)
state.set_traces(traces)


# PREPARING TESTING EXAMPLES
random.seed(142)
# create random 100 positions
N = 1000
test_positions = []
for i in range(N):
    state_tmp = state.deep_copy()
    state_tmp.set_random_state()
    test_positions.append(state_tmp)


# normal learning (without active, without implicit), WITH only positive
learner = meansends.TheoryConceptualizer(search_depth = SEARCH_DEPTH, active = False, m=M, 
                                         max_rules=5, att_alpha = 1.0, min_coverage=1, min_quality=0.0, 
                                         only_pure=False, max_conditions=6, only_positive=True, 
                                         only_positive_implicit=False, min_traces = MIN_TRACES, 
                                         max_branches=MAX_BRANCHES, add_implicit_conditions=False)
plan = learner(data, state)
plan.print_plan()
plan.save_to_file(open("sd%d_model_a_normal_pos_%d_%d_%4.2f.txt" % (
                        SEARCH_DEPTH, N, MIN_TRACES, COV_WEIGHT), "wt"))
# evaluating
evaluate.evaluate_gol(test_positions, "sd%d_model_a_normal_pos_%d_%d_%4.2f.txt" % (
                        SEARCH_DEPTH, N, MIN_TRACES, COV_WEIGHT), SEARCH_DEPTH, 1, state)


# learning with implicit conditions
learner = meansends.TheoryConceptualizer(search_depth = SEARCH_DEPTH, active = False, m=M, 
                                         max_rules=5, att_alpha = 1.0, min_coverage=1, min_quality=0.0, 
                                         only_pure=False, max_conditions=6, only_positive=True, 
                                         only_positive_implicit=False, min_traces = MIN_TRACES, 
                                         max_branches=MAX_BRANCHES, add_implicit_conditions=True)
plan = learner(data, state)
plan.print_plan()
plan.save_to_file(open("sd%d_model_a_implicit_%d_%d_%4.2f.txt" % (
                        SEARCH_DEPTH, N, MIN_TRACES, COV_WEIGHT), "wt")) 
# evaluating
evaluate.evaluate_gol(test_positions, "sd%d_model_a_implicit_%d_%d_%4.2f.txt" % (
                        SEARCH_DEPTH, N, MIN_TRACES, COV_WEIGHT), SEARCH_DEPTH, 1, state)

# active learning without implicit conditions
learner = meansends.TheoryConceptualizer(search_depth = SEARCH_DEPTH, active = True, m=M, 
                                         max_rules=5, att_alpha = 1.0, min_coverage=1, min_quality=0.0, 
                                         only_pure=False, max_conditions=6, only_positive=True, 
                                         only_positive_implicit=False, min_traces = MIN_TRACES, 
                                         max_branches=MAX_BRANCHES, add_implicit_conditions=False)
plan = learner(data, state)
plan.print_plan()
plan.save_to_file(open("sd%d_model_a_active_without_%d_%d_%4.2f.txt" % (
                        SEARCH_DEPTH, N, MIN_TRACES, COV_WEIGHT), "wt"))
# evaluating
evaluate.evaluate_gol(test_positions, "sd%d_model_a_active_without_%d_%d_%4.2f.txt" % (
                        SEARCH_DEPTH, N, MIN_TRACES, COV_WEIGHT), SEARCH_DEPTH, 1, state)


# active learning with implicit conditions
learner = meansends.TheoryConceptualizer(search_depth = SEARCH_DEPTH, active = True, m=M, 
                                         max_rules=5, att_alpha = 1.0, min_coverage=1, min_quality=0.0, 
                                         only_pure=False, max_conditions=6, only_positive=True, 
                                         only_positive_implicit=False, min_traces = MIN_TRACES, 
                                         max_branches=MAX_BRANCHES, add_implicit_conditions=True)
plan = learner(data, state)
plan.print_plan()
plan.save_to_file(open("sd%d_model_a_active_with_%d_%d_%4.2f.txt" % (
                        SEARCH_DEPTH, N, MIN_TRACES, COV_WEIGHT), "wt"))
# evaluating
evaluate.evaluate_gol(test_positions, "sd%d_model_a_active_with_%d_%d_%4.2f.txt" % (
                        SEARCH_DEPTH, N, MIN_TRACES, COV_WEIGHT), SEARCH_DEPTH, 1, state)
