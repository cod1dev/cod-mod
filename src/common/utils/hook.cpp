#include "hook.hpp"
#include "string.hpp"

#include <MinHook.h>

namespace utils::hook
{
    namespace
    {
        [[maybe_unused]] class _
        {
        public:
            _()
            {
                if (MH_Initialize() != MH_OK)
                {
                    throw std::runtime_error("Failed to initialize MinHook");
                }
            }

            ~_()
            {
                MH_Uninitialize();
            }
        } __;
    }

    void assembler::pushad()
    {
        
    }

    void assembler::popad()
    {
        
    }

    void assembler::prepare_stack_for_call()
    {
        
    }

    void assembler::restore_stack_after_call()
    {
        
    }

    asmjit::Error assembler::call(void* target)
    {
        return Assembler::call(size_t(target));
    }

    asmjit::Error assembler::jmp(void* target)
    {
        return Assembler::jmp(size_t(target));
    }

    detour::detour(const size_t place, void* target) : detour(reinterpret_cast<void*>(place), target)
    {
    }

    detour::detour(void* place, void* target)
    {
        this->create(place, target);
    }

    detour::~detour()
    {
        this->clear();
    }

    void detour::enable() const
    {
        MH_EnableHook(this->place_);
    }

    void detour::disable() const
    {
        MH_DisableHook(this->place_);
    }

    void detour::create(void* place, void* target)
    {
        this->clear();
        this->place_ = place;

        if (MH_CreateHook(this->place_, target, &this->original_) != MH_OK)
        {
            throw std::runtime_error(string::va("Unable to create hook at location: %p", this->place_));
        }

        this->enable();
    }

    void detour::create(const size_t place, void* target)
    {
        this->create(reinterpret_cast<void*>(place), target);
    }

    void detour::clear()
    {
        if (this->place_)
        {
            MH_RemoveHook(this->place_);
        }

        this->place_ = nullptr;
        this->original_ = nullptr;
    }

    void* detour::get_original() const
    {
        return this->original_;
    }

    bool iat(const nt::library& library, const std::string& target_library, const std::string& process, void* stub)
    {
        if (!library.is_valid()) return false;

        auto* const ptr = library.get_iat_entry(target_library, process);
        if (!ptr) return false;

        DWORD protect;
        VirtualProtect(ptr, sizeof(*ptr), PAGE_EXECUTE_READWRITE, &protect);

        *ptr = stub;

        VirtualProtect(ptr, sizeof(*ptr), protect, &protect);
        return true;
    }

    void nop(void* place, const size_t length)
    {
        DWORD old_protect{};
        VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &old_protect);

        std::memset(place, 0x90, length);

        VirtualProtect(place, length, old_protect, &old_protect);
        FlushInstructionCache(GetCurrentProcess(), place, length);
    }

    void nop(const size_t place, const size_t length)
    {
        nop(reinterpret_cast<void*>(place), length);
    }
    
    void call(void* pointer, void* data)
    {
        auto* patch_pointer = PBYTE(pointer);
        set<uint8_t>(patch_pointer, 0xE8);
        set<int32_t>(patch_pointer + 1, int32_t(size_t(data) - (size_t(pointer) + 5)));
    }

    void call(const size_t pointer, void* data)
    {
        return call(reinterpret_cast<void*>(pointer), data);
    }

    void call(const size_t pointer, const size_t data)
    {
        return call(pointer, reinterpret_cast<void*>(data));
    }
    
    void jump(void* pointer, void* data)
    {
        auto* patch_pointer = PBYTE(pointer);
        set<uint8_t>(patch_pointer, 0xE9);
        set<int32_t>(patch_pointer + 1, int32_t(size_t(data) - (size_t(pointer) + 5)));
    }

    void jump(const size_t pointer, void* data)
    {
        return jump(reinterpret_cast<void*>(pointer), data);
    }

    void jump(const size_t pointer, const size_t data)
    {
        return jump(pointer, reinterpret_cast<void*>(data));
    }

    void inject(void* pointer, const void* data)
    {
        set<int32_t>(pointer, int32_t(size_t(data) - (size_t(pointer) + 4)));
    }

    void inject(const size_t pointer, const void* data)
    {
        return inject(reinterpret_cast<void*>(pointer), data);
    }
}