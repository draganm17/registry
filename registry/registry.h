/*!
## Registry library version 0.92 ##
The Registry library provides facilities for performing operations on Windows registry and its components, such as keys
and values. The registry library was inspired by the <a href="http://en.cppreference.com/w/cpp/filesystem">Filesystem</a>
library and tries to mimic its design whenever possible.
*/

/* TODO:
    - symlinks support.
    - import / export of registry keys ?
    - transactions ?
*/

#pragma once

#include <registry/exception.h>
#include <registry/key.h>
#include <registry/key_handle.h>
#include <registry/key_iterator.h>
#include <registry/operations.h>
#include <registry/types.h>
#include <registry/value.h>
#include <registry/value_iterator.h>


//! Registry library namespace
namespace registry { }