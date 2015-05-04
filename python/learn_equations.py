import Orange
import equation.generate_states as eq_gol
#import equation.generate_states_2equations as eq_gol
import gol
import meansends

SEARCH_DEPTH = 1

state = eq_gol.Equations()
data = gol.create_data_from_all_positions(state)

learner = meansends.TheoryConceptualizer(search_depth = SEARCH_DEPTH, active = False, m=0, 
                                         max_rules=3, att_alpha = 1.0, min_coverage=1, min_quality=0.5, 
                                         only_pure=False, max_conditions=6, only_positive=False, min_traces = 0,
                                         max_branches=100, add_implicit_conditions=False)

plan = learner(data, state)
plan.print_plan()
