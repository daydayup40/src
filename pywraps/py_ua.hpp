#ifndef __PY_UA__
#define __PY_UA__

//-------------------------------------------------------------------------
//<code(py_ua)>
//</code(py_ua)>

//-------------------------------------------------------------------------
//<inline(py_ua)>
/*
#<pydoc>
def init_output_buffer(size = MAXSTR):
    """
    This function initialize an output buffer with the given size.
    It should be called before using any out_xxxx() functions.
    @return: It returns a string. This string should then be passed to MakeLine().
             This function could return None if it failed to create a buffer with the given size.
    """
    pass
#</pydoc>
*/
PyObject *py_init_output_buffer(size_t size = MAXSTR)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();

  // Let Python allocate a writable string buffer for us
  PyObject *py_str = PyString_FromStringAndSize(NULL, size);
  if ( py_str == NULL )
    Py_RETURN_NONE;

  init_output_buffer(PyString_AsString(py_str), size);
  return py_str;
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def term_output_buffer():
    """Use this function to terminate an output buffer."""
    pass
#</pydoc>
*/
void py_term_output_buffer()
{
  term_output_buffer();
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def decode_preceding_insn(ea):
    """
    Decodes the preceding instruction. Please check ua.hpp / decode_preceding_insn()
    @param ea: current ea
    @return: tuple(preceeding_ea or BADADDR, farref = Boolean)
    """
    pass
#</pydoc>
*/
PyObject *py_decode_preceding_insn(ea_t ea)
{
  bool farref;
  ea_t r = decode_preceding_insn(ea, &farref);
  PYW_GIL_CHECK_LOCKED_SCOPE();
  return Py_BuildValue("(" PY_FMT64 "i)", pyul_t(r), farref ? 1 : 0);
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def OutValue(op, outflags = 0):
    """
    Output immediate value
    @param op: operand (of type op_t)
    @return: flags of the output value
             -1: value is output with COLOR_ERROR
             0:  value is output as a number or character or segment
    """
    pass
#</pydoc>
*/
flags_t py_OutValue(PyObject *x, int outflags=0)
{
  op_t *op = op_t_get_clink(x);
  if ( op == NULL )
    return 0;

  return OutValue(*op, outflags);
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def OutImmChar(op, outflags = 0):
    """
    Output operand value as a commented character constant
    @param op: operand (of type op_t)
    @return: None
    """
    pass
#</pydoc>
*/
static void py_OutImmChar(PyObject *x)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *op = op_t_get_clink(x);
  if ( op != NULL )
    OutImmChar(*op);
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def ua_stkvar2(op, outflags = 0):
    """
    Create or modify a stack variable in the function frame.
    Please check ua.hpp / ua_stkvar2()
    @param op: operand (of type op_t)
    @return: None
    """
    pass
#</pydoc>
*/
static bool py_ua_stkvar2(PyObject *x, adiff_t v, int flags)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *op = op_t_get_clink(x);
  return op == NULL ? false : ua_stkvar2(*op, v, flags);
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def ua_add_off_drefs(op, type):
    """
    Add xrefs for offset operand of the current instruction
    Please check ua.hpp / ua_add_off_drefs()
    @param op: operand (of type op_t)
    @return: None
    """
    pass
#</pydoc>
*/
ea_t py_ua_add_off_drefs(PyObject *py_op, dref_t type)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *op = op_t_get_clink(py_op);
  return op == NULL ? BADADDR : ua_add_off_drefs(*op, type);
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def ua_add_off_drefs2(op, type, outf):
    """
    Add xrefs for offset operand of the current instruction
    Please check ua.hpp / ua_add_off_drefs2()
    @return: ea_t
    """
    pass
#</pydoc>
*/
ea_t py_ua_add_off_drefs2(PyObject *py_op, dref_t type, int outf)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *op = op_t_get_clink(py_op);
  return op == NULL ? BADADDR : ua_add_off_drefs2(*op, type, outf);
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def out_name_expr(op, ea, off):
    """
    Output a name expression
    @param op: operand (of type op_t)
    @param ea: address of expression
    @param off: the value of name expression. this parameter is used only to
                check that the name expression will have the wanted value.
                You may pass BADADDR for this parameter.
    @return: true if the name expression has been produced
    """
    pass
#</pydoc>
*/
bool py_out_name_expr(
  PyObject *py_op,
  ea_t ea,
  PyObject *py_off)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *op = op_t_get_clink(py_op);
  uint64 v(0);
  adiff_t off;
  if ( PyW_GetNumber(py_off, &v) )
    off = adiff_t(v);
  else
    off = BADADDR;

  return op == NULL ? false : out_name_expr(*op, ea, off);
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def construct_macro(insn):
    """
    See ua.hpp's construct_macro().
    """
    pass
#</pydoc>
*/
bool py_construct_macro(bool enable, PyObject *build_macro)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();

  if ( !PyCallable_Check(build_macro) )
    return false;

  static qstack<ref_t> macro_builders;

  macro_builders.push(newref_t(build_macro));
  struct ida_local lambda_t
  {
    static bool idaapi call_build_macro(insn_t &s, bool may_go_forward)
    {
      PyObject *py_builder = macro_builders.top().o;
      newref_t pyres = PyObject_CallFunction(
              py_builder, "O",
              may_go_forward ? Py_True : Py_False);
      PyW_ShowCbErr("build_macro");
      if ( pyres.o == NULL || pyres.o == Py_None )
        return false;
      insn_t *_s = insn_t_get_clink(pyres.o);
      if ( _s == NULL )
        return false;
      s = *_s;
      return true;
    }
  };
  bool res = construct_macro(enable, lambda_t::call_build_macro);
  macro_builders.pop();
  return res;
}

//-------------------------------------------------------------------------
static int py_get_dtyp_by_size(asize_t size)
{
  return int(get_dtyp_by_size(size));
}

//-------------------------------------------------------------------------
static PyObject *insn_t_get_op_link(PyObject *py_insn_lnk, int i)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  if ( i < 0 || i >= UA_MAXOP || !PyCObject_Check(py_insn_lnk) )
    Py_RETURN_NONE;

  // Extract C link
  insn_t *insn = (insn_t *)PyCObject_AsVoidPtr(py_insn_lnk);

  // Return a link to the operand
  return PyCObject_FromVoidPtr(&insn->Operands[i], NULL);
}

//-------------------------------------------------------------------------
static PyObject *insn_t_create()
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  return PyCObject_FromVoidPtr(new insn_t(), NULL);
}

//-------------------------------------------------------------------------
static PyObject *op_t_create()
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  return PyCObject_FromVoidPtr(new op_t(), NULL);
}

//-------------------------------------------------------------------------
static bool op_t_assign(PyObject *self, PyObject *other)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *lhs = op_t_get_clink(self);
  op_t *rhs = op_t_get_clink(other);
  if ( lhs == NULL || rhs == NULL )
    return false;

  *lhs = *rhs;
  return true;
}

//-------------------------------------------------------------------------
static bool insn_t_assign(PyObject *self, PyObject *other)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *lhs = insn_t_get_clink(self);
  insn_t *rhs = insn_t_get_clink(other);
  if ( lhs == NULL || rhs == NULL )
    return false;

  *lhs = *rhs;
  return true;
}

//-------------------------------------------------------------------------
static bool op_t_destroy(PyObject *py_obj)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  if ( !PyCObject_Check(py_obj) )
    return false;

  op_t *op = (op_t *)PyCObject_AsVoidPtr(py_obj);
  delete op;

  return true;
}

//-------------------------------------------------------------------------
static bool insn_t_destroy(PyObject *py_obj)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  if ( !PyCObject_Check(py_obj) )
    return false;

  delete (insn_t *)PyCObject_AsVoidPtr(py_obj);
  return true;
}

//-------------------------------------------------------------------------
// Returns a C link to the global 'cmd' variable
static PyObject *py_get_global_cmd_link()
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  return PyCObject_FromVoidPtr(&::cmd, NULL);
}

//-------------------------------------------------------------------------
static PyObject *insn_t_is_canon_insn(int itype)
{
  bool ok = ph.is_canon_insn(itype);
  PYW_GIL_CHECK_LOCKED_SCOPE();
  if ( ok )
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

//-------------------------------------------------------------------------
static PyObject *insn_t_get_canon_feature(int itype)
{
  uint32 v = ph.is_canon_insn(itype) ? ph.instruc[itype-ph.instruc_start].feature : 0;
  PYW_GIL_CHECK_LOCKED_SCOPE();
  return Py_BuildValue("I", v);
}

//-------------------------------------------------------------------------
static PyObject *insn_t_get_canon_mnem(int itype)
{
  bool ok = ph.is_canon_insn(itype);
  PYW_GIL_CHECK_LOCKED_SCOPE();
  if ( ok )
    return Py_BuildValue("s", ph.instruc[itype-ph.instruc_start].name);
  else
    Py_RETURN_NONE;
}

//-------------------------------------------------------------------------
static PyObject *insn_t_get_cs(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue(PY_FMT64, (pyul_t)link->cs);
}

static void insn_t_set_cs(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    return;

  uint64 v(0);
  PyW_GetNumber(value, &v);
  link->cs = ea_t(v);
}

static PyObject *insn_t_get_ip(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue(PY_FMT64, (pyul_t)link->ip);
}

static void insn_t_set_ip(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    return;
  uint64 v(0);
  PyW_GetNumber(value, &v);
  link->ip = ea_t(v);
}

static PyObject *insn_t_get_ea(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue(PY_FMT64, (pyul_t)link->ea);
}

static void insn_t_set_ea(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    return;
  uint64 v(0);
  PyW_GetNumber(value, &v);
  link->ea = ea_t(v);
}

static PyObject *insn_t_get_itype(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("H", link->itype);
}

static void insn_t_set_itype(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    return;
  link->itype = (uint16)PyInt_AsLong(value);
}

static PyObject *insn_t_get_size(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("H", link->size);
}

static void insn_t_set_size(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    return;
  link->size = (uint16)PyInt_AsLong(value);
}

static PyObject *insn_t_get_auxpref(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("H", link->auxpref);
}

static void insn_t_set_auxpref(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    return;
  link->auxpref = (uint16)PyInt_AsLong(value);
}

static PyObject *insn_t_get_segpref(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("b", link->segpref);
}

static void insn_t_set_segpref(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    return;
  link->segpref = (char)PyInt_AsLong(value);
}

static PyObject *insn_t_get_insnpref(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("b", link->insnpref);
}

static void insn_t_set_insnpref(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    return;
  link->insnpref = (char)PyInt_AsLong(value);
}

static PyObject *insn_t_get_flags(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("b", link->flags);
}

static void insn_t_set_flags(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  insn_t *link = insn_t_get_clink(self);
  if ( link == NULL )
    return;
  link->flags = (char)PyInt_AsLong(value);
}

//-------------------------------------------------------------------------
static PyObject *op_t_get_n(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("b", link->n);
}

static void op_t_set_n(PyObject *self, PyObject *value)
{
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  link->n = (char)PyInt_AsLong(value);
}

static PyObject *op_t_get_type(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("B", link->type);
}

static void op_t_set_type(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  link->type = (optype_t)PyInt_AsLong(value);
}

static PyObject *op_t_get_offb(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("b", link->offb);
}

static void op_t_set_offb(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  link->offb = (char)PyInt_AsLong(value);
}

static PyObject *op_t_get_offo(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("b", link->offo);
}

static void op_t_set_offo(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  link->offo = (char)PyInt_AsLong(value);
}

static PyObject *op_t_get_flags(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("B", link->flags);
}

static void op_t_set_flags(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  link->flags = (uchar)PyInt_AsLong(value);
}

static PyObject *op_t_get_dtyp(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("b", link->dtyp);
}

static void op_t_set_dtyp(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  link->dtyp = (char)PyInt_AsLong(value);
}

static PyObject *op_t_get_reg_phrase(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("H", link->reg);
}
static void op_t_set_reg_phrase(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  link->reg = (uint16)PyInt_AsLong(value);
}

static PyObject *op_t_get_value(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue(PY_FMT64, (pyul_t)link->value);
}

static void op_t_set_value(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  uint64 v(0);
  PyW_GetNumber(value, &v);
  link->value = uval_t(v);
}

static PyObject *op_t_get_addr(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue(PY_FMT64, (pyul_t)link->addr);
}

static void op_t_set_addr(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  uint64 v(0);
  PyW_GetNumber(value, &v);
  link->addr = ea_t(v);
}

static PyObject *op_t_get_specval(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue(PY_FMT64, (pyul_t)link->specval);
}

static void op_t_set_specval(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  uint64 v(0);
  PyW_GetNumber(value, &v);
  link->specval = ea_t(v);
}

static PyObject *op_t_get_specflag1(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("b", link->specflag1);
}

static void op_t_set_specflag1(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  link->specflag1 = (char)PyInt_AsLong(value);
}

static PyObject *op_t_get_specflag2(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("b", link->specflag2);
}

static void op_t_set_specflag2(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  link->specflag2 = (char)PyInt_AsLong(value);
}

static PyObject *op_t_get_specflag3(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("b", link->specflag3);
}

static void op_t_set_specflag3(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  link->specflag3 = (char)PyInt_AsLong(value);
}

static PyObject *op_t_get_specflag4(PyObject *self)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    Py_RETURN_NONE;
  return Py_BuildValue("b", link->specflag4);
}

static void op_t_set_specflag4(PyObject *self, PyObject *value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();
  op_t *link = op_t_get_clink(self);
  if ( link == NULL )
    return;
  link->specflag4 = (char)PyInt_AsLong(value);
}

//-------------------------------------------------------------------------
PyObject *py_get_operand_immvals(ea_t ea, int n)
{
  uvalvec_t storage;
  storage.resize(2 * UA_MAXOP);
  size_t cnt = get_operand_immvals(ea, n, storage.begin());
  PyObject *result = PyList_New(cnt);
  for ( size_t i = 0; i < cnt; ++i )
    PyList_SetItem(result, i, Py_BuildValue(PY_FMT64, pyul_t(storage[i])));
  return result;
}

//</inline(py_ua)>

#endif
