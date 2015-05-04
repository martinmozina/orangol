#ifndef __CALLBACK_GOL_HPP
#define __CALLBACK_GOL_HPP

#include "Python.h"
#include "cls_orange.hpp"
#include "cls_value.hpp"
#include "px/orangol_globals.hpp"
#include "px/externs.px"
#include "GOLDomain.hpp"

ORANGOL_API PyObject *callCallback(PyObject *self, PyObject *args);
ORANGOL_API PyObject *setCallbackFunction(PyObject *self, PyObject *func);

ORANGOL_API PyObject *callbackReduce(PyObject *self, TOrangeType &basetype);


class ORANGOL_API TGOL_State_Python : public TGOL_State {
public:
  __REGISTER_CLASS
  virtual double evaluate() const;
  virtual PGOL_MoveList getMoves() const;
  virtual void doMove(const PGOL_Move & move);
  virtual void undoMove(const PGOL_Move & move);
  virtual double pruneProb(const PGOL_State &, const PGOL_GoalList &, const int & maxDepth, const int & depth) const;
  virtual bool orNode() const;
  virtual TValue & getAttributeValue(const int & position) const;
  virtual PGOL_State deepCopy() const;
  virtual string id(void) const;
};

class ORANGOL_API TGOL_Move_Python : public TGOL_Move {
public:
  __REGISTER_CLASS
  virtual string toString() const;
};

#endif
