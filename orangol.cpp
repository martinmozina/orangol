
#include "orange.hpp"
#include "px/orangol_globals.hpp"
#include "px/externs.px"
#include "cls_orange.hpp"
#include "vectortemplates.hpp"
#include "callback.hpp"

#include "GOLDomain.hpp"
#include "GOLSearcher.hpp"

extern ORANGOL_API TOrangeType PyOrGOL_SubGoal_Type;
extern ORANGOL_API TOrangeType PyOrGOL_Goal_Type;
extern ORANGOL_API TOrangeType PyOrGOL_Move_Type;
extern ORANGOL_API TOrangeType PyOrGOL_State_Type;
extern ORANGOL_API TOrangeType PyOrGOL_EvaluateResult_Type;

// Goal-oriented learning (GOL)
C_NAMED(GOL_Goal - Orange.GOL_Goal, Orange, "()");
PyObject *GOL_Goal_call(PyObject *self, PyObject *args, PyObject *keywords) PYDOC("(state, state) -/-> (boolean)")
{
  PyTRY
    NO_KEYWORDS

    PGOL_State s1, s2;

    if (!PyArg_ParseTuple(args, "O&O&:GOL_Goal.call", cc_GOL_State, &s1, cc_GOL_State, &s2))
      return PYNULL;
    CAST_TO(TGOL_Goal, g)

    return PyInt_FromLong((*g)(s1, s2));
  PyCATCH
}

PyObject *GOL_Goal_str(TPyOrange *self)
{
  PyObject *result = callbackOutput((PyObject *)self, NULL, NULL, "str", "repr");
  if (result)
    return result;

  CAST_TO(TGOL_Goal, er);
  return PyString_FromString(er->toString().c_str());
}

C_NAMED(GOL_SubGoal - Orange.GOL_SubGoal, Orange, "(position,operator,holding_goal,domain[, values])");
PyObject *GOL_SubGoal_new(PyObject *self, PyObject *args, PyObject *keywords) PYDOC("(position, operator, holding_goal, domain[, values])")
{
  PyTRY
    NO_KEYWORDS

    if (PyOrange_OrangeBaseClass(self->ob_type) == &PyOrGOL_SubGoal_Type) {
      PyErr_Format(PyExc_SystemError, "GOL_SubGoal.call called for '%s': this may lead to stack overflow", self->ob_type->tp_name);
      return PYNULL;
    }

    int position = -1;
    int operi = 0;
    bool holdingGoal = false;
    PValueList values;
    PDomain domain;

//    if (!PyArg_ParseTuple(args, "iiO&b|O&:GOL_SubGoal.new", &position, &operi, &holdingGoal, cc_ValueList, &values))
//      return PYNULL;
    if (!PyArg_ParseTuple(args, "iibO&|O&:GOL_SubGoal.new", &position, &operi, &holdingGoal, cc_Domain, &domain, cc_ValueList, &values))
      return PYNULL;

    TGOL_SubGoal::Operator oper = (TGOL_SubGoal::Operator)operi;
    TGOL_SubGoal *sg = new TGOL_SubGoal(position, oper, holdingGoal, domain, values);
    PGOL_SubGoal wsg = sg;
    return WrapOrange(wsg);
  PyCATCH
}
PyObject *GOL_SubGoal_call(PyObject *self, PyObject *args, PyObject *keywords) PYDOC("(state, state) -/-> (boolean)")
{
  PyTRY
    NO_KEYWORDS

    PGOL_State s1, s2;

    if (!PyArg_ParseTuple(args, "O&O&:GOL_SubGoal.call", cc_GOL_State, &s1, cc_GOL_State, &s2))
      return PYNULL;
    CAST_TO(TGOL_SubGoal, sg)

    return PyInt_FromLong((*sg)(s1, s2));
  PyCATCH
}

C_NAMED(GOL_ANDORSearcher - Orange.GOL_ANDORSearcher, Orange, "()");
C_NAMED(GOL_EvaluateResult - Orange.GOL_EvaluateResult, Orange, "()");

PyObject *GOL_EvaluateResult_str(TPyOrange *self)
{
  PyObject *result = callbackOutput((PyObject *)self, NULL, NULL, "str", "repr");
  if (result)
    return result;

  CAST_TO(TGOL_EvaluateResult, er);
  return PyString_FromString(er->toString().c_str());
}


PyObject *GOL_ANDORSearcher_evaluateGoals(PyObject *self, PyObject *args, PyObject *keywords) PYARGS(METH_VARARGS, "(state,moves,goals,depth) -> evaluate_result")
{
  PyTRY
    PGOL_State start;
    PGOL_MoveList moves;
    PGOL_GoalList goals;
    int maxDepth;
    bool storeIDs;

    if (!PyArg_ParseTuple(args, "O&O&O&ib:GOL_ANDORSearcher.evaluateGoals", cc_GOL_State, &start, cc_GOL_MoveList, &moves, cc_GOL_GoalList, &goals, &maxDepth, &storeIDs))
      return PYNULL;
    CAST_TO(TGOL_ANDORSearcher, searcher)

    return WrapOrange(searcher->evaluateGoals(start, moves, goals, maxDepth, storeIDs));
  PyCATCH
}

PyObject *GOL_ANDORSearcher_evaluateGoalsSet(PyObject *self, PyObject *args, PyObject *keywords) PYARGS(METH_VARARGS, "(state,moves,goals,depth) -> evaluate_result_list")
{
  PyTRY
    PGOL_State start;
    PGOL_MoveList moves;
    PGOL_GoalList goals;
    int maxDepth;

    if (!PyArg_ParseTuple(args, "O&O&O&i:GOL_ANDORSearcher.evaluateGoalsSet", cc_GOL_State, &start, cc_GOL_MoveList, &moves, cc_GOL_GoalList, &goals, &maxDepth))
      return PYNULL;
    CAST_TO(TGOL_ANDORSearcher, searcher)

    return WrapOrange(searcher->evaluateGoalsSet(start, moves, goals, maxDepth));
  PyCATCH
}

PyObject *GOL_ANDORSearcher_assignGoals(PyObject *self, PyObject *args, PyObject *keywords) PYARGS(METH_VARARGS, "(state, max_depth, max_subgoals, k) -> achievable_goals")
{
  PyTRY
    PGOL_State state;
    PGOL_GoalList start = PGOL_GoalList();
    int maxDepth, maxSubGoals, K;
    bool specialize = true;
    if (!PyArg_ParseTuple(args, "O&iii|O&b:GOL_ANDORSearcher.assignGoals", cc_GOL_State, &state, &maxDepth, &maxSubGoals, &K, cc_GOL_GoalList, &start, &specialize))
      return PYNULL;
    CAST_TO(TGOL_ANDORSearcher, searcher)

    TGOL_GoalList * ua = mlnew TGOL_GoalList();
    PGOL_GoalList unachievable = ua;

    PGOL_EvaluateResultList goodGoals = searcher->assignGoals(state, maxDepth, maxSubGoals, K, start, specialize, unachievable);
    return Py_BuildValue("NN", WrapOrange(goodGoals), WrapOrange(unachievable));
  PyCATCH
}

ABSTRACT(GOL_Move - Orange.GOL_Move, Orange);
PyObject *GOL_Move_new(PyTypeObject *type, PyObject *args, PyObject *keywords)  BASED_ON(Orange, "<abstract>")
{ if (type == (PyTypeObject *)&PyOrGOL_Move_Type)
    return setCallbackFunction(WrapNewOrange(mlnew TGOL_Move_Python(), type), args);
  else
    return WrapNewOrange(mlnew TGOL_Move_Python(), type);
}
PyObject *GOL_Move__reduce__(PyObject *self)
{
  return callbackReduce(self, PyOrGOL_Move_Type);
}

ABSTRACT(GOL_State - Orange.GOL_State, Orange);
PyObject *GOL_State_new(PyTypeObject *type, PyObject *args, PyObject *keywords)  BASED_ON(Orange, "<abstract>")
{ if (type == (PyTypeObject *)&PyOrGOL_State_Type)
    return setCallbackFunction(WrapNewOrange(mlnew TGOL_State_Python(), type), args);
  else
    return WrapNewOrange(mlnew TGOL_State_Python(), type);
}
PyObject *GOL_State__reduce__(PyObject *self)
{
  return callbackReduce(self, PyOrGOL_State_Type);
}


PGOL_SubGoalList PGOL_SubGoalList_FromArguments(PyObject *arg) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::P_FromArguments(arg); }
PyObject *GOL_SubGoalList_FromArguments(PyTypeObject *type, PyObject *arg) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_FromArguments(type, arg); }
PyObject *GOL_SubGoalList_new(PyTypeObject *type, PyObject *arg, PyObject *kwds) BASED_ON(Orange, "(<list of GOL_SubGoal>)") ALLOWS_EMPTY { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_new(type, arg, kwds); }
PyObject *GOL_SubGoalList_getitem_sq(TPyOrange *self, Py_ssize_t index) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_getitem(self, index); }
int       GOL_SubGoalList_setitem_sq(TPyOrange *self, Py_ssize_t index, PyObject *item) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_setitem(self, index, item); }
PyObject *GOL_SubGoalList_getslice(TPyOrange *self, Py_ssize_t start, Py_ssize_t stop) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_getslice(self, start, stop); }
int       GOL_SubGoalList_setslice(TPyOrange *self, Py_ssize_t start, Py_ssize_t stop, PyObject *item) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_setslice(self, start, stop, item); }
Py_ssize_t       GOL_SubGoalList_len_sq(TPyOrange *self) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_len(self); }
PyObject *GOL_SubGoalList_richcmp(TPyOrange *self, PyObject *object, int op) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_richcmp(self, object, op); }
PyObject *GOL_SubGoalList_concat(TPyOrange *self, PyObject *obj) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_concat(self, obj); }
PyObject *GOL_SubGoalList_repeat(TPyOrange *self, Py_ssize_t times) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_repeat(self, times); }
PyObject *GOL_SubGoalList_str(TPyOrange *self) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_str(self); }
PyObject *GOL_SubGoalList_repr(TPyOrange *self) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_str(self); }
int       GOL_SubGoalList_contains(TPyOrange *self, PyObject *obj) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_contains(self, obj); }
PyObject *GOL_SubGoalList_append(TPyOrange *self, PyObject *item) PYARGS(METH_O, "(GOL_SubGoal) -> None") { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_append(self, item); }
PyObject *GOL_SubGoalList_extend(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(sequence) -> None") { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_extend(self, obj); }
PyObject *GOL_SubGoalList_count(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_SubGoal) -> int") { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_count(self, obj); }
PyObject *GOL_SubGoalList_filter(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "([filter-function]) -> GOL_SubGoalList") { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_filter(self, args); }
PyObject *GOL_SubGoalList_index(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_SubGoal) -> int") { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_index(self, obj); }
PyObject *GOL_SubGoalList_insert(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "(index, item) -> None") { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_insert(self, args); }
PyObject *GOL_SubGoalList_native(TPyOrange *self) PYARGS(METH_NOARGS, "() -> list") { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_native(self); }
PyObject *GOL_SubGoalList_pop(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "() -> GOL_SubGoal") { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_pop(self, args); }
PyObject *GOL_SubGoalList_remove(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_SubGoal) -> None") { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_remove(self, obj); }
PyObject *GOL_SubGoalList_reverse(TPyOrange *self) PYARGS(METH_NOARGS, "() -> None") { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_reverse(self); }
PyObject *GOL_SubGoalList_sort(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "([cmp-func]) -> None") { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_sort(self, args); }
PyObject *GOL_SubGoalList__reduce__(TPyOrange *self, PyObject *) { return ListOfWrappedMethods<PGOL_SubGoalList, TGOL_SubGoalList, PGOL_SubGoal, &PyOrGOL_SubGoal_Type>::_reduce(self); }

PGOL_GoalList PGOL_GoalList_FromArguments(PyObject *arg) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::P_FromArguments(arg); }
PyObject *GOL_GoalList_FromArguments(PyTypeObject *type, PyObject *arg) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_FromArguments(type, arg); }
PyObject *GOL_GoalList_new(PyTypeObject *type, PyObject *arg, PyObject *kwds) BASED_ON(Orange, "(<list of GOL_Goal>)") ALLOWS_EMPTY { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_new(type, arg, kwds); }
PyObject *GOL_GoalList_getitem_sq(TPyOrange *self, Py_ssize_t index) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_getitem(self, index); }
int       GOL_GoalList_setitem_sq(TPyOrange *self, Py_ssize_t index, PyObject *item) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_setitem(self, index, item); }
PyObject *GOL_GoalList_getslice(TPyOrange *self, Py_ssize_t start, Py_ssize_t stop) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_getslice(self, start, stop); }
int       GOL_GoalList_setslice(TPyOrange *self, Py_ssize_t start, Py_ssize_t stop, PyObject *item) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_setslice(self, start, stop, item); }
Py_ssize_t       GOL_GoalList_len_sq(TPyOrange *self) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_len(self); }
PyObject *GOL_GoalList_richcmp(TPyOrange *self, PyObject *object, int op) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_richcmp(self, object, op); }
PyObject *GOL_GoalList_concat(TPyOrange *self, PyObject *obj) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_concat(self, obj); }
PyObject *GOL_GoalList_repeat(TPyOrange *self, Py_ssize_t times) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_repeat(self, times); }
PyObject *GOL_GoalList_str(TPyOrange *self) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_str(self); }
PyObject *GOL_GoalList_repr(TPyOrange *self) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_str(self); }
int       GOL_GoalList_contains(TPyOrange *self, PyObject *obj) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_contains(self, obj); }
PyObject *GOL_GoalList_append(TPyOrange *self, PyObject *item) PYARGS(METH_O, "(GOL_Goal) -> None") { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_append(self, item); }
PyObject *GOL_GoalList_extend(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(sequence) -> None") { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_extend(self, obj); }
PyObject *GOL_GoalList_count(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_Goal) -> int") { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_count(self, obj); }
PyObject *GOL_GoalList_filter(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "([filter-function]) -> GOL_GoalList") { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_filter(self, args); }
PyObject *GOL_GoalList_index(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_Goal) -> int") { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_index(self, obj); }
PyObject *GOL_GoalList_insert(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "(index, item) -> None") { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_insert(self, args); }
PyObject *GOL_GoalList_native(TPyOrange *self) PYARGS(METH_NOARGS, "() -> list") { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_native(self); }
PyObject *GOL_GoalList_pop(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "() -> GOL_Goal") { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_pop(self, args); }
PyObject *GOL_GoalList_remove(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_Goal) -> None") { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_remove(self, obj); }
PyObject *GOL_GoalList_reverse(TPyOrange *self) PYARGS(METH_NOARGS, "() -> None") { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_reverse(self); }
PyObject *GOL_GoalList_sort(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "([cmp-func]) -> None") { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_sort(self, args); }
PyObject *GOL_GoalList__reduce__(TPyOrange *self, PyObject *) { return ListOfWrappedMethods<PGOL_GoalList, TGOL_GoalList, PGOL_Goal, &PyOrGOL_Goal_Type>::_reduce(self); }

PGOL_MoveList PGOL_MoveList_FromArguments(PyObject *arg) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::P_FromArguments(arg); }
PyObject *GOL_MoveList_FromArguments(PyTypeObject *type, PyObject *arg) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_FromArguments(type, arg); }
PyObject *GOL_MoveList_new(PyTypeObject *type, PyObject *arg, PyObject *kwds) BASED_ON(Orange, "(<list of GOL_Move>)") ALLOWS_EMPTY { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_new(type, arg, kwds); }
PyObject *GOL_MoveList_getitem_sq(TPyOrange *self, Py_ssize_t index) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_getitem(self, index); }
int       GOL_MoveList_setitem_sq(TPyOrange *self, Py_ssize_t index, PyObject *item) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_setitem(self, index, item); }
PyObject *GOL_MoveList_getslice(TPyOrange *self, Py_ssize_t start, Py_ssize_t stop) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_getslice(self, start, stop); }
int       GOL_MoveList_setslice(TPyOrange *self, Py_ssize_t start, Py_ssize_t stop, PyObject *item) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_setslice(self, start, stop, item); }
Py_ssize_t       GOL_MoveList_len_sq(TPyOrange *self) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_len(self); }
PyObject *GOL_MoveList_richcmp(TPyOrange *self, PyObject *object, int op) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_richcmp(self, object, op); }
PyObject *GOL_MoveList_concat(TPyOrange *self, PyObject *obj) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_concat(self, obj); }
PyObject *GOL_MoveList_repeat(TPyOrange *self, Py_ssize_t times) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_repeat(self, times); }
PyObject *GOL_MoveList_str(TPyOrange *self) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_str(self); }
PyObject *GOL_MoveList_repr(TPyOrange *self) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_str(self); }
int       GOL_MoveList_contains(TPyOrange *self, PyObject *obj) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_contains(self, obj); }
PyObject *GOL_MoveList_append(TPyOrange *self, PyObject *item) PYARGS(METH_O, "(GOL_Move) -> None") {return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_append(self, item);}
PyObject *GOL_MoveList_extend(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(sequence) -> None") { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_extend(self, obj); }
PyObject *GOL_MoveList_count(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_Move) -> int") { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_count(self, obj); }
PyObject *GOL_MoveList_filter(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "([filter-function]) -> GOL_MoveList") { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_filter(self, args); }
PyObject *GOL_MoveList_index(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_Move) -> int") { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_index(self, obj); }
PyObject *GOL_MoveList_insert(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "(index, item) -> None") { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_insert(self, args); }
PyObject *GOL_MoveList_native(TPyOrange *self) PYARGS(METH_NOARGS, "() -> list") { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_native(self); }
PyObject *GOL_MoveList_pop(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "() -> GOL_Move") { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_pop(self, args); }
PyObject *GOL_MoveList_remove(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_Move) -> None") { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_remove(self, obj); }
PyObject *GOL_MoveList_reverse(TPyOrange *self) PYARGS(METH_NOARGS, "() -> None") { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_reverse(self); }
PyObject *GOL_MoveList_sort(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "([cmp-func]) -> None") { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_sort(self, args); }
PyObject *GOL_MoveList__reduce__(TPyOrange *self, PyObject *) { return ListOfWrappedMethods<PGOL_MoveList, TGOL_MoveList, PGOL_Move, &PyOrGOL_Move_Type>::_reduce(self); }

PGOL_EvaluateResultList PGOL_EvaluateResultList_FromArguments(PyObject *arg) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::P_FromArguments(arg); }
PyObject *GOL_EvaluateResultList_FromArguments(PyTypeObject *type, PyObject *arg) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_FromArguments(type, arg); }
PyObject *GOL_EvaluateResultList_new(PyTypeObject *type, PyObject *arg, PyObject *kwds) BASED_ON(Orange, "(<list of GOL_EvaluateResult>)") ALLOWS_EMPTY { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_new(type, arg, kwds); }
PyObject *GOL_EvaluateResultList_getitem_sq(TPyOrange *self, Py_ssize_t index) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_getitem(self, index); }
int       GOL_EvaluateResultList_setitem_sq(TPyOrange *self, Py_ssize_t index, PyObject *item) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_setitem(self, index, item); }
PyObject *GOL_EvaluateResultList_getslice(TPyOrange *self, Py_ssize_t start, Py_ssize_t stop) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_getslice(self, start, stop); }
int       GOL_EvaluateResultList_setslice(TPyOrange *self, Py_ssize_t start, Py_ssize_t stop, PyObject *item) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_setslice(self, start, stop, item); }
Py_ssize_t       GOL_EvaluateResultList_len_sq(TPyOrange *self) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_len(self); }
PyObject *GOL_EvaluateResultList_richcmp(TPyOrange *self, PyObject *object, int op) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_richcmp(self, object, op); }
PyObject *GOL_EvaluateResultList_concat(TPyOrange *self, PyObject *obj) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_concat(self, obj); }
PyObject *GOL_EvaluateResultList_repeat(TPyOrange *self, Py_ssize_t times) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_repeat(self, times); }
PyObject *GOL_EvaluateResultList_str(TPyOrange *self) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_str(self); }
PyObject *GOL_EvaluateResultList_repr(TPyOrange *self) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_str(self); }
int       GOL_EvaluateResultList_contains(TPyOrange *self, PyObject *obj) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_contains(self, obj); }
PyObject *GOL_EvaluateResultList_append(TPyOrange *self, PyObject *item) PYARGS(METH_O, "(GOL_EvaluateResult) -> None") {return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_append(self, item);}
PyObject *GOL_EvaluateResultList_extend(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(sequence) -> None") { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_extend(self, obj); }
PyObject *GOL_EvaluateResultList_count(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_EvaluateResult) -> int") { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_count(self, obj); }
PyObject *GOL_EvaluateResultList_filter(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "([filter-function]) -> GOL_EvaluateResultList") { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_filter(self, args); }
PyObject *GOL_EvaluateResultList_index(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_EvaluateResult) -> int") { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_index(self, obj); }
PyObject *GOL_EvaluateResultList_insert(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "(index, item) -> None") { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_insert(self, args); }
PyObject *GOL_EvaluateResultList_native(TPyOrange *self) PYARGS(METH_NOARGS, "() -> list") { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_native(self); }
PyObject *GOL_EvaluateResultList_pop(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "() -> GOL_EvaluateResult") { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_pop(self, args); }
PyObject *GOL_EvaluateResultList_remove(TPyOrange *self, PyObject *obj) PYARGS(METH_O, "(GOL_EvaluateResult) -> None") { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_remove(self, obj); }
PyObject *GOL_EvaluateResultList_reverse(TPyOrange *self) PYARGS(METH_NOARGS, "() -> None") { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_reverse(self); }
PyObject *GOL_EvaluateResultList_sort(TPyOrange *self, PyObject *args) PYARGS(METH_VARARGS, "([cmp-func]) -> None") { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_sort(self, args); }
PyObject *GOL_EvaluateResultList__reduce__(TPyOrange *self, PyObject *) { return ListOfWrappedMethods<PGOL_EvaluateResultList, TGOL_EvaluateResultList, PGOL_EvaluateResult, &PyOrGOL_EvaluateResult_Type>::_reduce(self); }


bool initorangolExceptions()
{ return true; }

void gcorangolUnsafeStaticInitialization()
{}

ORANGOL_API PyObject *orangolModule;

#include "px/initialization.px"

#include "px/orangol.px"


