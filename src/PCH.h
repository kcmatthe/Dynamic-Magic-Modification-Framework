#pragma once

#include "RE/Skyrim.h"
#include <REL/Relocation.h>
#include "SKSE/SKSE.h"

#define WIN32_LEAN_AND_MEAN
#define NOMMNOSOUND

#include <SimpleIni.h>
#include <robin_hood.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <xbyak/xbyak.h>


namespace logger = SKSE::log;

using namespace std::literals;

namespace stl
{
	using namespace SKSE::stl;

	void asm_replace(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to);

	template <class T>
	void asm_replace(std::uintptr_t a_from)
	{
		asm_replace(a_from, T::size, reinterpret_cast<std::uintptr_t>(T::func));
	}

	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);

		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}

	template <class T>
	void write_thunk_branch(std::uintptr_t a_src)
	{
		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);

		T::func = trampoline.write_branch<5>(a_src, T::thunk);
	}

	inline void write_14branch(std::uintptr_t a_src, const void* a_code)
	{
		#pragma pack(push, 1)
		struct Assembly
		{
			// jmp [rip]
			std::uint8_t jmp;    // 0 - 0xFF
			std::uint8_t modrm;  // 1 - 0x25
			std::int32_t disp;   // 2 - 0x00000000
			std::uint64_t addr;  // 6 - [rip]
		};
		static_assert(offsetof(Assembly, jmp) == 0x0);
		static_assert(offsetof(Assembly, modrm) == 0x1);
		static_assert(offsetof(Assembly, disp) == 0x2);
		static_assert(offsetof(Assembly, addr) == 0x6);
		static_assert(sizeof(Assembly) == 0xE);
		#pragma pack(pop)

		Assembly mem{
			.jmp = 0xFF,
			.modrm = 0x25,
			.disp = 0x00000000,
			.addr = std::bit_cast<std::uint64_t>(a_code),
		};

		REL::safe_write(a_src, std::addressof(mem), sizeof(mem));
	}



	template <class F, size_t offset, class T>
	void write_vfunc()
	{
		REL::Relocation<std::uintptr_t> vtbl{ F::VTABLE[offset] };
		T::func = vtbl.write_vfunc(T::idx, T::thunk);
	}

	template <class F, class T>
	void write_vfunc()
	{
		write_vfunc<F, 0, T>();
	}

	inline std::string as_string(std::string_view a_view)
	{
		return { a_view.data(), a_view.size() };
	}
}

#ifdef SKYRIM_AE
#	define OFFSET(se, ae) ae
#else
#	define OFFSET(se, ae) se
#endif

#define DLLEXPORT __declspec(dllexport)

#include "Version.h"
