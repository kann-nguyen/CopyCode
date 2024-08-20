#include <windows.h>

int main() {
    CHAR moduleName[128] = "";
    GetModuleFileNameA(NULL, moduleName, sizeof(moduleName));
    MessageBoxA(NULL, moduleName, "Message Box", MB_OK);
    return 0;
}
