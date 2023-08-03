#include <iostream>

#include "hv.h"
#include "dumper.h"

inline UINT32 GetTextHashW(wchar_t* Str) {

    UINT32 Hash = NULL;

    while (Str != NULL && *Str) {

        Hash = (UINT32)(65599 * (Hash + (*Str++) + (*Str > 64 && *Str < 91 ? 32 : 0)));
    }

    return Hash;
}

int main() {
  if (!hv::is_hv_running()) {
    printf("HV not running.\n");
    return 0;
  }

  auto const hv_base = static_cast<uint8_t*>(hv::get_hv_base());
  auto const hv_size = 0x64000;

  // hide the hypervisor
  hv::for_each_cpu([&](uint32_t) {
    for (size_t i = 0; i < hv_size; i += 0x1000) {
      auto const virt = hv_base + i;
      auto const phys = hv::get_physical_address(0, virt);

      if (!phys) {
        printf("failed to get physical address for 0x%p.\n", virt);
        continue;
      }

      if (!hv::hide_physical_page(phys >> 12))
        printf("failed to hide page: 0x%p.\n", virt);
    }
  });

  //ULONG64 base = hv::query_module_base(3040, GetTextHashW((wchar_t*)L"Kernel32.dll"));

  //printf("query Kernel32 module base is 0x%p", base);

  printf("Pinged the hypervisor! Flushing logs...\n");

  FILE* file = nullptr;
  fopen_s(&file, "hvlog.txt", "a");

  while (!GetAsyncKeyState(VK_RETURN)) {
    // flush the logs
    uint32_t count = 512;
    hv::logger_msg msgs[512];
    hv::flush_logs(count, msgs);

    // print the logs
    for (uint32_t i = 0; i < count; ++i) {
      printf("[%I64u][CPU=%u] %s\n", msgs[i].id, msgs[i].aux, msgs[i].data);
      fprintf(file, "[%I64u][CPU=%u] %s\n", msgs[i].id, msgs[i].aux, msgs[i].data);
    }

    fflush(file);
    Sleep(1);
  }

  Sleep(3000);

  hv::key_act((USHORT)MapVirtualKey(VK_NUMPAD5, 0), 0);
  fclose(file);

  hv::for_each_cpu([](uint32_t) {
    hv::remove_all_mmrs();
  });
}

