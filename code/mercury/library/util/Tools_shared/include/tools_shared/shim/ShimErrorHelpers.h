#ifndef SHIM_ERROR_HELPERS_H
#define SHIM_ERROR_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

#define BEGIN_ALWAYS { {
#define BEGIN_CHECK_SUCCESS { if (!success) {
#define BEGIN_CHECK_NOTNULL(value) { if (value == 0) {
#define BEGIN_CHECK_CUSTOM(error) { if (error) {

#define ERROR_XML_ATTR(root) ERROR_FATAL("Could not find required XML attribute in %s.\n", (root)->GetName().Chars())
#define ERROR_XML_NODE(name) ERROR_FATAL("Could not find XML node %s.\n", name)

#define ERROR_FATAL(error, ...) {LOGFATAL(Tools::g_pMyApplication->GetLogger(), error, __VA_ARGS__); EXECUTER_FATAL(error, __VA_ARGS__);}
#define ERROR_ERROR(error, ...) {LOGERROR(Tools::g_pMyApplication->GetLogger(), error, __VA_ARGS__); EXECUTER_ERROR(error, __VA_ARGS__);}
#define ERROR_WARN(error, ...) {LOGWARN(Tools::g_pMyApplication->GetLogger(), error, __VA_ARGS__); EXECUTER_WARN(error, __VA_ARGS__);}
#define ERROR_INFO(error, ...) {LOGINFO(Tools::g_pMyApplication->GetLogger(), error, __VA_ARGS__); EXECUTER_INFO(error, __VA_ARGS__);}
#define ERROR_DEBUG(error, ...) {LOGDEBUG(Tools::g_pMyApplication->GetLogger(), error, __VA_ARGS__);}

#define END_DO_NOTHING } }
#define END_GOTO_EXIT goto exit; } }
#define END_RETURN_FALSE	return false; } }
#define END_RETURN return; } }

#endif