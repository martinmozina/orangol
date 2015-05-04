#include "callback.ppp"

PyObject *callCallback(PyObject *self, PyObject *args)
{
    PyObject *result;

    if (PyObject_HasAttrString(self, "__callback")) {
        PyObject *callback = PyObject_GetAttrString(self, "__callback");
        result = PyObject_CallObject(callback, args);
        Py_DECREF(callback);
    }
    else
        result = PyObject_CallObject(self, args);

    if (!result)
        throw pyexception();

    return result;
}

PyObject *callMethod(char const *method, PyObject *self, PyObject *args)
{

    if (PyObject_HasAttrString(self, const_cast<char *>(method))) {
        PyObject *callback = PyObject_GetAttrString(self, const_cast<char *>(method));
        PyObject *result = PyObject_CallObject(callback, args);
        Py_DECREF(callback);

        if (!result)
            throw pyexception();

        return result;
    }

    raiseErrorWho("Python object does not provide method '%s'", method);
    return NULL; // to make the compiler happy
}

PyObject *setCallbackFunction(PyObject *self, PyObject *args)
{
    PyObject *func;
    if (!PyArg_ParseTuple(args, "O", &func)) {
        PyErr_Format(PyExc_TypeError, "callback function for '%s' expected",
                self->ob_type->tp_name);
        Py_DECREF(self);
        return PYNULL;
    }
    else if (!PyCallable_Check(func)) {
        PyErr_Format(PyExc_TypeError, "'%s' object is not callable",
                func->ob_type->tp_name);
        Py_DECREF(self);
        return PYNULL;
    }

    PyObject_SetAttrString(self, "__callback", func);
    return self;
}

PyObject *callbackReduce(PyObject *self, TOrangeType &basetype)
{
    if (self->ob_type == (PyTypeObject *) &basetype) {
        PyObject *packed = packOrangeDictionary(self);
        PyObject *callback = PyDict_GetItemString(packed, "__callback");
        if (!callback)
            PYERROR(
                    PyExc_AttributeError,
                    "cannot pickle an invalid callback object ('__callback' attribute is missing)",
                    NULL);

        PyDict_DelItemString(packed, "__callback");
        return Py_BuildValue("O(O)N", self->ob_type, callback, packed);
    }
    else
        return Py_BuildValue("O()N", self->ob_type, packOrangeDictionary(self));
}


double TGOL_State_Python::evaluate(void) const
{
    PyObject *args = Py_BuildValue("()");
    PyObject *result = callMethod("evaluate", (PyObject *) myWrapper, args);
    Py_DECREF(args);

    if (!PyFloat_Check(result)) {
        raiseError("__call__ is expected to return a float value.");
    }

    double res = PyFloat_AsDouble(result);
    Py_DECREF(result);
    return res;
}

PGOL_MoveList TGOL_State_Python::getMoves(void) const
{
    PyObject *args = Py_BuildValue("()");
    PyObject *result = callMethod(PyObject_HasAttrString((PyObject *)myWrapper, "get_moves") ? "get_moves" : "getMoves",
                                 (PyObject *) myWrapper, args);
    Py_DECREF(args);

    if (!PyOrGOL_MoveList_Check(result))
      raiseError("GOL_State.getMoves must return a MoveList");
    PGOL_MoveList ml = PyOrange_AsGOL_MoveList(result);
    Py_DECREF(result);
    return ml;
}

void TGOL_State_Python::doMove(const PGOL_Move & move) 
{
    PyObject *args = Py_BuildValue("(N)", WrapOrange(move));
    callMethod(PyObject_HasAttrString((PyObject *)myWrapper, "do_move") ? "do_move" : "doMove",
                                 (PyObject *) myWrapper, args);
    Py_DECREF(args);
}

void TGOL_State_Python::undoMove(const PGOL_Move & move) 
{
    PyObject *args = Py_BuildValue("(N)", WrapOrange(move));
    callMethod(PyObject_HasAttrString((PyObject *)myWrapper, "undo_move") ? "undo_move" : "undoMove",
                                 (PyObject *) myWrapper, args);
    Py_DECREF(args);
}

double TGOL_State_Python::pruneProb(const PGOL_State & state, const PGOL_GoalList & gl, const int & maxDepth, const int & depth) const 
{
    PyObject *args = Py_BuildValue("(NNii)", WrapOrange(state), WrapOrange(gl), depth);
    PyObject *result = callMethod(PyObject_HasAttrString((PyObject *)myWrapper, "prune_prob") ? "prune_prob" : "pruneProb",
                                 (PyObject *) myWrapper, args);
    Py_DECREF(args);
    if (!PyFloat_Check(result)) {
        raiseError("pruneProb is expected to return a Float value.");
    }

    double res = PyFloat_AsDouble(result);
    Py_DECREF(result);
    return res;
}

bool TGOL_State_Python::orNode() const 
{
    PyObject *args = Py_BuildValue("()");
    PyObject *result = callMethod(PyObject_HasAttrString((PyObject *)myWrapper, "or_node") ? "or_node" : "orNode",
                                 (PyObject *) myWrapper, args);
    Py_DECREF(args);
    if (!PyBool_Check(result)) {
        raiseError("orNode is expected to return a Boolean value.");
    }

    bool os = bool(PyObject_IsTrue(result)!=0);
    Py_DECREF(result);
    return os;
}


PGOL_State TGOL_State_Python::deepCopy() const
{   
    PyObject *result = PyObject_CallMethod((PyObject *) myWrapper,
            PyObject_HasAttrString((PyObject *)myWrapper, "deep_copy") ? "deep_copy" : "deepCopy",
            NULL);
    if (!result)
        raiseError("An exception has been thrown in method deepCopy!");
    if (!PyOrGOL_State_Check(result))
        raiseError(
                "deepCopy is expected to return an instance of a class derived from GOL_State");

    PGOL_State gs = PyOrange_AsGOL_State(result);
    Py_DECREF(result);
    return gs;
}

TValue & TGOL_State_Python::getAttributeValue(const int & position) const 
{
    PyObject *args = Py_BuildValue("(i)", position);
    PyObject *result = callMethod(PyObject_HasAttrString((PyObject *)myWrapper, "get_attribute_value") ? "get_attribute_value" : "getAttributeValue",
                                 (PyObject *) myWrapper, args);
    Py_DECREF(args);
    if (!PyOrValue_Check(result)) {
        raiseError("pruneProb is expected to return an Orange Value.");
    }

    TValue & value = PyValue_AS_Value(result);
    Py_DECREF(result);
    return value;
}

string TGOL_State_Python::id(void) const {
    PyObject *result = PyObject_CallMethod((PyObject *) myWrapper, "id", NULL);
    if (!result)
        raiseError("An exception has been thrown in method id!");
    if (!PyString_Check(result))
        raiseError("id is expected to return an instance of a string.");

    string ts = PyString_AsString(result);
    Py_DECREF(result);
    return ts;
}


string TGOL_Move_Python::toString(void) const {
    PyObject *result = PyObject_CallMethod((PyObject *) myWrapper,
            PyObject_HasAttrString((PyObject *)myWrapper, "__str__") ? "__str__" : "toString",
            NULL);
    if (!result)
        raiseError("An exception has been thrown in method toString!");
    if (!PyString_Check(result))
        raiseError("toString is expected to return an instance of a string.");

    string ts = PyString_AsString(result);
    Py_DECREF(result);
    return ts;
};