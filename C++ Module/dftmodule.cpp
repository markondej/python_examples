#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <vector>
#include <complex>
#include <cstring>
#include <cmath>

class FFT {
public:
    FFT() = delete;
    FFT(const FFT &) = delete;
    FFT(FFT &&) = delete;
    FFT &operator=(const FFT &) = delete;
    static std::vector<std::complex<double>> Perform(const std::vector<std::complex<double>> &input, bool inverse = false) {
        std::size_t stride = 1, size = input.size();
        auto offset = GenInputOrder(size >> 1);
        std::vector<std::complex<double>> output;
        output.resize(size);

        while (size > 2) {
            stride <<= 1;
            size >>= 1;
        }

        for (std::size_t i = 0; i < stride; i++) {
            Radix2FFT(&input[offset[i]], &output[i * 2], stride);
        }

        while (stride > 1) {
            stride >>= 1;
            size <<= 1;
            for (std::size_t i = 0; i < stride; i++) {
                Sum2FFT(&output[i * size], &output[i * size], size >> 1, inverse);
            }
        }

        if (inverse) {
            for (std::size_t i = 0; i < size; i++) {
                output[i] /= static_cast<double>(size);
            }
        }

        return output;
    }
private:
    static void Radix2FFT(const std::complex<double> *input, std::complex<double> *output, std::size_t stride) {
        output[0].real(input[0].real() + input[stride].real());
        output[0].imag(input[0].imag() + input[stride].imag());

        output[1].real(input[0].real() - input[stride].real());
        output[1].imag(input[0].imag() - input[stride].imag());
    }
    static void Sum2FFT(const std::complex<double> *input, std::complex<double> *output, std::size_t size, bool inverse) {
        double dfi = (inverse ? 2.0 : -2.0) * M_PI / static_cast<double>(size << 1),
            kfi = 0.0;

        for (std::size_t k = 0; k < size; k++) {
            double cosfi = cos(kfi), sinfi = sin(kfi);
            std::complex<double> temp[] = { input[k], input[k + size] };

            output[k].real(temp[0].real() + cosfi * temp[1].real() - sinfi * temp[1].imag());
            output[k].imag(temp[0].imag() + sinfi * temp[1].real() + cosfi * temp[1].imag());
            output[k + size].real(temp[0].real() - cosfi * temp[1].real() + sinfi * temp[1].imag());
            output[k + size].imag(temp[0].imag() - sinfi * temp[1].real() - cosfi * temp[1].imag());
            kfi += dfi;
        }
    }
    static std::vector<std::size_t> GenInputOrder(std::size_t size) {
        std::size_t stride = 1;
        std::vector<std::size_t> offset;
        offset.resize(size);

        std::memset(offset.data(), 0xff, sizeof(std::size_t) * size);
        offset[0] = 0;

        for (std::size_t step = size >> 1; step > 0; step >>= 1) {
            std::size_t base = 0;
            for (std::size_t i = 0; i < size; i += step) {
                if (offset[i] != SIZE_MAX) {
                    base = offset[i];
                }
                else {
                    offset[i] = base + stride;
                }
            }
            stride <<= 1;
        }

        return offset;
    }
};

static PyObject *FFTError;

static PyObject *DFT_FFT(PyObject *self, PyObject *args)
{
    PyObject *object;
    if (!PyArg_ParseTuple(args, "O", &object)) {
        return NULL;
    }

    std::vector<std::complex<double>> input;
    try {
        PyObject *iterator = PyObject_GetIter(object);
        if (iterator) {
            while (true) {
                PyObject *real = PyIter_Next(iterator);
                if (!real) {
                    break;
                }
                PyObject *imag = PyIter_Next(iterator);
                if (!imag || !PyFloat_Check(real) || !PyFloat_Check(imag)) {
                    throw std::exception();
                }
                input.push_back({ PyFloat_AsDouble(real), PyFloat_AsDouble(imag) });
            }
        }

        if (input.empty() || (input.size() & (input.size() - 1))) {
            throw std::exception();
        }
    } catch (...) {
        PyErr_SetString(FFTError, "Input must be list of double precision numbers with size of power of 2");
        return NULL;
    }

    auto output = FFT::Perform(input);

    PyObject* list = PyList_New(output.size() << 1);
    for (std::size_t i = 0; i < output.size(); ++i) {
        PyObject* real = Py_BuildValue("d", output[i].real());
        PyObject* imag = Py_BuildValue("d", output[i].imag());
        PyList_SetItem(list, i << 1, real);
        PyList_SetItem(list, (i << 1) + 1, imag);
    }

    return list;
}

static PyMethodDef DFTMethods[] = {
    { "fft",  DFT_FFT, METH_VARARGS, "Performs Fast Fourier Transform" },
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef DFTModule = {
    PyModuleDef_HEAD_INIT,
    "dft",
    "Discrete Fourier Transform module",
    -1,
    DFTMethods
};

PyMODINIT_FUNC PyInit_dft(void) {
    PyObject *mod = PyModule_Create(&DFTModule);
    if (mod != NULL) {
        FFTError = PyErr_NewException("dft.fft_error", NULL, NULL);
        Py_XINCREF(FFTError);
        if (PyModule_AddObject(mod, "error", FFTError) < 0) {
            Py_XDECREF(FFTError);
            Py_CLEAR(FFTError);
            Py_DECREF(mod);
            return NULL;
        }
    }
    return mod;
}
