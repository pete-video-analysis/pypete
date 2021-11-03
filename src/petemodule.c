#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <analysis.h>

static PyObject *request_next_frame, *notify_flash, *notify_over_three_flashes;

void _request_next_frame()
{
	if(request_next_frame == NULL) return;
	PyObject_CallObject(request_next_frame, NULL);
}

void _notify_flash(FLASH* flash, uint16_t x, uint16_t y, bool is_red)
{
	if(notify_flash == NULL) return;
	PyObject *dict = Py_BuildValue(
		"({s:i,"
		"s:i,"
		"s:i,"
		"s:i,"
		"s:O})",
		"start_frame", flash->start_frame,
		"end_frame", flash->end_frame,
		"x", x,
		"y", y,
		"is_red", is_red ? Py_True : Py_False
	);
	PyObject_CallObject(notify_flash, dict);
	Py_DECREF(dict);
}

void _notify_over_three_flashes(uint64_t start, uint64_t end, uint16_t x, uint16_t y, bool is_red)
{
	if(notify_over_three_flashes == NULL) return;
	PyObject *dict = Py_BuildValue(
		"({s:i,"
		"s:i,"
		"s:i,"
		"s:i,"
		"s:O})",
		"start_frame", start,
		"end_frame", end,
		"x", x,
		"y", y,
		"is_red", is_red ? Py_True : Py_False
	);
	PyObject_CallObject(notify_over_three_flashes, dict);
	Py_DECREF(dict);
}

static PyObject *set_next_frame_callback(PyObject *dummy, PyObject *args)
{
	PyObject *result = NULL;

	PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp)) {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(temp);         /* Add a reference to new callback */
        Py_XDECREF(request_next_frame);  /* Dispose of previous callback */
        request_next_frame = temp;       /* Remember new callback */
        
        pete_request_next_frame = _request_next_frame;
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

static PyObject *set_flash_callback(PyObject *dummy, PyObject *args)
{
	PyObject *result = NULL;

	PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp)) {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(temp);         /* Add a reference to new callback */
        Py_XDECREF(notify_flash);  /* Dispose of previous callback */
        notify_flash = temp;       /* Remember new callback */
        
        pete_notify_flash = _notify_flash;

        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

static PyObject *set_over_three_flashes_callback(PyObject *dummy, PyObject *args)
{
	PyObject *result = NULL;

	PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp)) {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(temp);         /* Add a reference to new callback */
        Py_XDECREF(notify_over_three_flashes);  /* Dispose of previous callback */
        notify_over_three_flashes = temp;       /* Remember new callback */
        
        pete_notify_over_three_flashes = _notify_over_three_flashes;

        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

static PyObject *set_metadata(PyObject *dummy, PyObject *args)
{
	int _width, _height, _fps, _has_alpha;

	if(PyArg_ParseTuple(args, "iiii", &_width, &_height, &_fps, &_has_alpha))
	{
		uint16_t width = _width;
		uint16_t height = _height;
		uint8_t fps = _fps;
		bool has_alpha = (_has_alpha == 1);

		pete_set_metadata(width, height, fps, has_alpha);
	}
	else
	{
		fprintf(stderr, "Something went wrong while setting the metadata\n");
	}

	Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *send_frame(PyObject *dummy, PyObject *args)
{
	uint8_t *data;
	Py_buffer *buf;

	if(PyArg_ParseTuple(args, "y*", buf))
	{
		data = buf->buf;
		pete_receive_frame(data);
		PyBuffer_Release(buf);
	}

	Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef PeteMethods[] = {
	{"set_next_frame_callback", set_next_frame_callback, METH_VARARGS, "Set the request next frame callback"},
	{"set_flash_callback", set_flash_callback, METH_VARARGS, "Set the notify flash callback"},
	{"set_over_three_flashes_callback", set_over_three_flashes_callback, METH_VARARGS, "Set the notify over three flashes callback"},
	{"set_metadata", set_metadata, METH_VARARGS, "Set metadata"},
	{"send_frame", send_frame, METH_VARARGS, "Send frame"},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef petemodule = {
	PyModuleDef_HEAD_INIT,
	"pete",
	NULL,
	-1,
	PeteMethods
};

PyMODINIT_FUNC PyInit_pete(void)
{
	return PyModule_Create(&petemodule);
}