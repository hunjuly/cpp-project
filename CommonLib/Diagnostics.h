#include "stdafx.h"

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define __WFUNCTION__ WIDEN(__FUNCTION__)

#define THROW() (throw Exceptions::LibraryException(__WFILE__, __WFUNCTION__, __LINE__))
#define THROW_LIBRARY(message) (throw Exceptions::LibraryException(__WFILE__, __WFUNCTION__, __LINE__, message))
#define CHECK_NULL(value, message) {if(!value) THROW_LIBRARY(message);}
#define CHECK_NULL_THROW_LAST_ERROR(value) {CHECK_NULL(value, Diagnostics::GetLastErrorMessage());}
#define CHECK_HRESULT(value, message) {if(FAILED(value)) THROW_LIBRARY(message);}
#define CHECKHR(value) {if(FAILED(value)) THROW();}
#define CHECK_RETURN(value) {if(FAILED(value)) return value;}
