"""
Evaluation of a goal - strategy. 
"""
import random
import goaltree
import collections

def evaluate_gol(positions, filename, search_depth, random_steps, state, max_steps = 1000):
    print "EVALUATING: " + filename
    gt = goaltree.create_goal_tree_from_file(open(filename, "rt"), search_depth, state)
    
    # loop through random position
    nsolved = 0.
    nsteps = 0.
    dtg, sdtg = 0., 0.
    found = 0
    relevant_branches = collections.defaultdict(int)
    for i, state in enumerate(positions):
        # first select best branch
        relbranch = None
        branches = []
        steps = 0
        state = state.deep_copy()
        start_dtg = state.evaluate()
        while not relbranch and steps < max_steps:
        #for irs in range(random_steps):
            relbranch, relevant = gt.get_best_branch(state, search_depth)
            if not relbranch:
                moves = state.get_moves()
                rmove = random.choice(moves)
                state.do_move(rmove)
                steps += 1
            else:
                for r in relevant:
                    relevant_branches[r] += 1     
   
        
        solved = False
        while not solved and steps < max_steps:
            # get goal
            goal = None
            plan, (new_state, new_steps), ev = relbranch.get_best_goal(state, search_depth)
            if plan:
                goal = plan.goals
            if not goal:
                # do a random step but keep in rule
                moves = state.get_moves()
                rmove = random.choice(moves)
                state.do_move(rmove)
                steps += 1
            else:
                state = new_state
                steps += new_steps
            if state.evaluate() == 0:
                solved = True 
    
        print "final steps: ", steps
        if steps < max_steps:
            nsolved += 1
            sdtg += start_dtg
            nsteps += steps
            found += 1
        print "solv% :", nsolved / (i+1), "steps: ", nsteps / float(found+1)
        if relevant_branches:
            print "most often seen branch: "
            most_relevant = max(relevant_branches.keys(), key = lambda x: relevant_branches[x])
            print most_relevant
            most_relevant_branch = gt.select_branch_by_indices(most_relevant)
            most_relevant_branch.print_plan()
        
    
    print "Percentage solved: ", nsolved / len(positions)
    print "Found: ", found
    if nsolved > 0:
        print "Average steps: ", nsteps / nsolved #float(found)
    else:
        print "Average steps: NA"
    print "Start dtg: ", sdtg / nsolved
    
