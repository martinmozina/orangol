import re
import Orange
import orangol

operators = [orangol.GOL_SubGoal.NoChange,
             orangol.GOL_SubGoal.Increase,
             orangol.GOL_SubGoal.Decrease,
             orangol.GOL_SubGoal.NonIncrease,
             orangol.GOL_SubGoal.NonDecrease,
             orangol.GOL_SubGoal.Equals,
             orangol.GOL_SubGoal.GreaterThan,
             orangol.GOL_SubGoal.LessThan,
             orangol.GOL_SubGoal.External]

class GOLVariable(Orange.core.PythonVariable):
    """ For writing and parsing goals in .tab files. """
    def str2val(self, strV):
        """ convert str to val - used for creating variables. """
        return self.filestr2val(strV, None)

    def filestr2val(self, strV, example=None):
        """ The structure of a set of goals int the .tab format is: goal1:complexity1;goal2:complexity2; ...
        Each goal can be a combination of several subgoals, therefore the structure of a goal is goal=subgoal1,subgoal2,...
        """

        gocom = strV.strip().split(";")
        gocom_list = [v.split(":") for v in gocom]

        if not gocom_list:
            return Orange.core.PythonValueSpecial(2)

        if not example:
            return strV
        
        goals = []
        for gcm in gocom_list:
            goal = orangol.GOL_Goal()
            goal.subGoals = orangol.GOL_SubGoalList()
            for sgcm in gcm[0].split(","):
                params = re.search(r"\[(.*)\]", sgcm).groups()[0].strip().split("|")
                params = map(eval, params)

                vals = Orange.core.ValueList()
                for v in params[3:]:
                    if isinstance(example.domain.attributes[params[0]], Orange.feature.Discrete):
                        vals.append(Orange.core.Value(example.domain.attributes[params[0]], int(v)))
                    else:
                        vals.append(Orange.core.Value(example.domain.attributes[params[0]], float(v)))
                subgoal = orangol.GOL_SubGoal(params[0], operators[params[1]], params[2], example.domain, vals)
                goal.subGoals.append(subgoal)
            if len(gcm)>1:
                complexity = float(gcm[1]) # set complexity always, set -1 if unknown
            else:
                complexity = -1
            goals.append((goal,complexity))
        return goals

    # used for writing to data: specify output (string) presentation of arguments in tab. file
    def val2filestr(self, val, example=None):
        """ val should be a list of pairs: (Orange.core.GOL_Goal, complexity) """
        return ";".join("%s:%4.3f"%(str(v[0]),v[1]) for v in val)

    # used for writing to string
    def val2str(self, val):
        return self.val2filestr(val)


    