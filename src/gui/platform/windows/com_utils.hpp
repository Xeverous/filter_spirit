#pragma once

#include "platform/windows/generic_utils.hpp"

#include <Windows.h>
#include <objbase.h>

#include <string>
#include <string_view>
#include <memory>

struct iunknown_deleter
{
	void operator()(IUnknown* p) const
	{
		if (p != nullptr)
			p->Release();
	}
};

template <typename T>
using com_interface_ptr = std::unique_ptr<T, iunknown_deleter>;

struct co_task_deleter
{
	void operator()(LPVOID pv) const
	{
		CoTaskMemFree(pv);
	}
};

template <typename T>
using co_task_ptr = std::unique_ptr<T, co_task_deleter>;

template <typename T>
com_interface_ptr<T> make_instance(REFCLSID rclsid)
{
	T* p = nullptr;
	// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
	// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-iid_ppv_args
	HRESULT hr = CoCreateInstance(rclsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&p));

	if (!SUCCEEDED(hr))
		throw_windows_error(hr, "CoCreateInstance");

	return com_interface_ptr<T>(p);
}

