#pragma once
#include <memory>
#include <plog/Log.h>

#include "globals.hpp"
#include "lib_recomp.hpp"

#include "controllers/lifetime_controller.hpp"
#include "controllers/global_root_controller.hpp"
#include "controllers/thread_root_controller.hpp"

extern std::unique_ptr<LifetimeController> l_controller;
extern GlobalRootController* g_controller;
extern thread_local ThreadRootController* t_controller;

#define RECOMP_ARG_PYOBJECT(pos) t_controller->get_py_object(RECOMP_ARG(REPY_Handle, pos))