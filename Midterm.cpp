// Leena Jahed
// DFOR 740 Midterm

#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <winsvc.h>
#include <stdio.h>
#include <string>
#include <vector> //Allocate dynamic buffers for structures
#include <map> //Store optional parameters for create and config commands

// Windows API Functions require wide-character strings so wmain is used
int wmain(int argc, wchar_t* argv[]) {

    //Provide user command options
    if (argc < 2) {
        wprintf(L"Usage: %ls <command> [options]\n", argv[0]);
        wprintf(L"Commands: query, qc, create, qdescription, start, stop, delete, config, failure, qfailure\n");
        return 1;
    }

    //Store the command argument
    std::wstring cmd = argv[1];

    // Query Command – show service status or list all services
    if (cmd == L"query") {
        //Open Service Control Manager
        SC_HANDLE sch = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
        // Specific service information if service is found
        if (argc >= 3) {
            // If service is found, open the service with the permission to obtain query information
            SC_HANDLE sh = OpenService(sch, argv[2], SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
            SERVICE_STATUS_PROCESS ssp;
            DWORD bytesNeeded;
            QueryServiceStatusEx(sh, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &bytesNeeded);
            DWORD bufSize = 0;
            GetServiceDisplayName(sch, argv[2], nullptr, &bufSize);
            std::wstring displayName(bufSize, L'\0');
            GetServiceDisplayName(sch, argv[2], &displayName[0], &bufSize);
            displayName.resize(wcslen(displayName.c_str()));
            //Print service information
            wprintf(L"\nSERVICE_NAME: %ls\n", argv[2]);
            wprintf(L"   TYPE: %lu\n", ssp.dwServiceType);
            wprintf(L"   STATE: %lu\n", ssp.dwCurrentState);
            wprintf(L"   WIN32_EXIT_CODE: %lu\n", ssp.dwWin32ExitCode);
            wprintf(L"   SERVICE_EXIT_CODE: %lu\n", ssp.dwServiceSpecificExitCode);
            wprintf(L"   CHECKPOINT: %lu\n", ssp.dwCheckPoint);
            wprintf(L"   WAIT_HINT: %lu\n", ssp.dwWaitHint);
            wprintf(L"   PROCESS_ID: %lu\n", ssp.dwProcessId);
            wprintf(L"   DISPLAY_NAME: %ls\n", displayName.c_str());
           
            CloseServiceHandle(sh);
        }
        // List all services
        else {
            DWORD bytesNeeded = 0, servicesReturned = 0, resumeHandle = 0;
            std::vector<BYTE> buffer;
            //Obtain service list
            EnumServicesStatusEx(sch, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                nullptr, 0, &bytesNeeded, &servicesReturned, &resumeHandle, nullptr);
            buffer.resize(bytesNeeded);
            EnumServicesStatusEx(sch, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                buffer.data(), (DWORD)buffer.size(), &bytesNeeded,
                &servicesReturned, &resumeHandle, nullptr);
            //Put the services into an array and loop print them
            LPENUM_SERVICE_STATUS_PROCESS services = (LPENUM_SERVICE_STATUS_PROCESS)buffer.data();
            wprintf(L"\nSERVICE_NAME: (all)\n");
            for (DWORD i = 0; i < servicesReturned; i++)
                wprintf(L"    %ls\n", services[i].lpServiceName);
        }
        CloseServiceHandle(sch);
    }


    // QC Command – query configuration details
    else if (cmd == L"qc") {
        //Open Service Control Manager
        SC_HANDLE sch = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        //Open service with configuration permission
        SC_HANDLE sh = OpenService(sch, argv[2], SERVICE_QUERY_CONFIG);
        DWORD bytesNeeded = 0;
        QueryServiceConfig(sh, nullptr, 0, &bytesNeeded);
        std::vector<BYTE> buffer(bytesNeeded);
        LPQUERY_SERVICE_CONFIG qsc = (LPQUERY_SERVICE_CONFIG)buffer.data();
        QueryServiceConfig(sh, qsc, bytesNeeded, &bytesNeeded);
        //Print configuration fields
        wprintf(L"\n[SC] QueryServiceConfig SUCCESS\n\n");
        wprintf(L"SERVICE_NAME: %ls\n", argv[2]);
        wprintf(L"   TYPE: %lu\n", qsc->dwServiceType);
        wprintf(L"   START_TYPE: %lu\n", qsc->dwStartType);
        wprintf(L"   ERROR_CONTROL: %lu\n", qsc->dwErrorControl);
        wprintf(L"   BINARY_PATH_NAME: %ls\n", qsc->lpBinaryPathName);
        wprintf(L"   LOAD_ORDER_GROUP: %ls\n", qsc->lpLoadOrderGroup);
        wprintf(L"   TAG: %lu\n", qsc->dwTagId);
        wprintf(L"   DISPLAY_NAME: %ls\n", qsc->lpDisplayName);
        wprintf(L"   DEPENDENCIES: %ls\n", qsc->lpDependencies);
        wprintf(L"   SERVICE_START_NAME: %ls\n", qsc->lpServiceStartName);
        CloseServiceHandle(sh);
        CloseServiceHandle(sch);
    }


    // Create Command – install a new service
    else if (cmd == L"create") {
        //Store the command argument
        const wchar_t* serviceName = argv[2];
        //Parse optional parameters to find binary path for the new service and store the key
        std::map<std::wstring, std::wstring> opts;
        for (int i = 3; i < argc; i++) {
            wchar_t* eq = wcschr(argv[i], L'=');
            if (eq && i + 1 < argc) {
                std::wstring key(argv[i], eq - argv[i]);
                opts[key] = argv[i + 1];
                i++;
            }
        }
        //Open Service Control Manager with create permission
        SC_HANDLE sch = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        DWORD dwStartType = SERVICE_DEMAND_START;
        DWORD dwErrorControl = SERVICE_ERROR_NORMAL;
        std::wstring binPath = opts[L"binPath"];

        std::wstring displayName = opts[L"DisplayName"];
        std::wstring dependencies = opts[L"depend"];
        std::wstring account = opts[L"obj"];
        std::wstring password = opts[L"password"];

        //Override the defaults if options were provided in command
        if (opts[L"type"] == L"share") dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
        if (opts[L"start"] == L"auto") dwStartType = SERVICE_AUTO_START;
        if (opts[L"start"] == L"disabled") dwStartType = SERVICE_DISABLED;
        if (opts[L"error"] == L"severe") dwErrorControl = SERVICE_ERROR_SEVERE;
        if (opts[L"error"] == L"critical") dwErrorControl = SERVICE_ERROR_CRITICAL;
        if (opts[L"error"] == L"ignore") dwErrorControl = SERVICE_ERROR_IGNORE;
        //Create service with all paremeters filled and print success
        SC_HANDLE sh = CreateService(sch, serviceName,
            displayName.empty() ? nullptr : displayName.c_str(),
            SERVICE_ALL_ACCESS, dwServiceType, dwStartType, dwErrorControl,
            binPath.c_str(), nullptr, nullptr,
            dependencies.empty() ? nullptr : dependencies.c_str(),
            account.empty() ? nullptr : account.c_str(),
            password.empty() ? nullptr : password.c_str());
        wprintf(L"CreateService Successful!\n");
        CloseServiceHandle(sh);
        CloseServiceHandle(sch);
    }


    // Qdescription Command – retrieve service description
    else if (cmd == L"qdescription") {
        //Open Service Control Manager
        SC_HANDLE sch = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        //Open service with config permissions
        SC_HANDLE sh = OpenService(sch, argv[2], SERVICE_QUERY_CONFIG);
        DWORD bytesNeeded = 0;
        QueryServiceConfig2(sh, SERVICE_CONFIG_DESCRIPTION, nullptr, 0, &bytesNeeded);
        std::vector<BYTE> buffer(bytesNeeded);
        LPSERVICE_DESCRIPTION sd = (LPSERVICE_DESCRIPTION)buffer.data();
        //Retrieve description information and print
        QueryServiceConfig2(sh, SERVICE_CONFIG_DESCRIPTION, buffer.data(), bytesNeeded, &bytesNeeded);
        wprintf(L"\n QueryServiceConfig2 Successful!\n\n");
        wprintf(L"SERVICE_NAME: %ls\n", argv[2]);
        wprintf(L"   DESCRIPTION: %ls\n", sd->lpDescription ? sd->lpDescription : L"");
        CloseServiceHandle(sh);
        CloseServiceHandle(sch);
    }


    // Start Command – start a service
    else if (cmd == L"start") {
        //Open Service Control Manager
        SC_HANDLE sch = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        //Open service with start permissions
        SC_HANDLE sh = OpenService(sch, argv[2], SERVICE_START);
        //Start the Service and print success
        StartService(sh, 0, nullptr);
        wprintf(L"StartService Successful!\n");
        CloseServiceHandle(sh);
        CloseServiceHandle(sch);
    }


    // Stop Command – stop a service
    else if (cmd == L"stop") {
        //Open Service Control Manager
        SC_HANDLE sch = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        //Open service with stop permissions
        SC_HANDLE sh = OpenService(sch, argv[2], SERVICE_STOP);
        SERVICE_STATUS ss;
        //Stop the service and print success
        ControlService(sh, SERVICE_CONTROL_STOP, &ss);
        wprintf(L"ControlService STOP Successful!\n");
        CloseServiceHandle(sh);
        CloseServiceHandle(sch);
    }


    // Delete Command – remove a service
    else if (cmd == L"delete") {
        //Open Service Control Manager
        SC_HANDLE sch = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        //Open service with delete  permissions
        SC_HANDLE sh = OpenService(sch, argv[2], DELETE);
        //Deletes the service and print success
        DeleteService(sh);
        wprintf(L"DeleteService Successful!\n");
        CloseServiceHandle(sh);
        CloseServiceHandle(sch);
    }


    // Config Command – modify service configuration (type, start, binary path, etc.)
    else if (cmd == L"config") {
        //Store the command argument
        const wchar_t* serviceName = argv[2];
        //Open Service Control Manager
        SC_HANDLE sch = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        //Open Service with config permissions
        SC_HANDLE sh = OpenService(sch, serviceName, SERVICE_CHANGE_CONFIG);
        //Parse through parameter options and retreive current config to store in key
        std::map<std::wstring, std::wstring> opts;
        for (int i = 3; i < argc; i++) {
            wchar_t* eq = wcschr(argv[i], L'=');
            if (eq && i + 1 < argc) {
                std::wstring key(argv[i], eq - argv[i]);
                opts[key] = argv[i + 1];
                i++;
            }
        }
        DWORD bytesNeeded = 0;
        QueryServiceConfig(sh, nullptr, 0, &bytesNeeded);
        std::vector<BYTE> buffer(bytesNeeded);
        LPQUERY_SERVICE_CONFIG qsc = (LPQUERY_SERVICE_CONFIG)buffer.data();
        QueryServiceConfig(sh, qsc, bytesNeeded, &bytesNeeded);
        //Initialize with current config values
        DWORD dwServiceType = qsc->dwServiceType;
        DWORD dwStartType = qsc->dwStartType;
        DWORD dwErrorControl = qsc->dwErrorControl;
        std::wstring binPath = qsc->lpBinaryPathName;
        std::wstring loadOrderGroup = qsc->lpLoadOrderGroup;
        std::wstring dependencies = qsc->lpDependencies;
        std::wstring account = qsc->lpServiceStartName;
        std::wstring displayName = qsc->lpDisplayName;
        //Override current config values if presented in command
        if (opts[L"type"] == L"own") dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        if (opts[L"type"] == L"share") dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
        if (opts[L"start"] == L"auto") dwStartType = SERVICE_AUTO_START;
        if (opts[L"start"] == L"demand") dwStartType = SERVICE_DEMAND_START;
        if (opts[L"start"] == L"disabled") dwStartType = SERVICE_DISABLED;
        if (opts[L"error"] == L"normal") dwErrorControl = SERVICE_ERROR_NORMAL;
        if (opts[L"error"] == L"severe") dwErrorControl = SERVICE_ERROR_SEVERE;
        if (opts[L"error"] == L"critical") dwErrorControl = SERVICE_ERROR_CRITICAL;
        if (opts[L"error"] == L"ignore") dwErrorControl = SERVICE_ERROR_IGNORE;
        if (opts.find(L"binPath") != opts.end()) binPath = opts[L"binPath"];
        if (opts.find(L"group") != opts.end()) loadOrderGroup = opts[L"group"];
        if (opts.find(L"depend") != opts.end()) dependencies = opts[L"depend"];
        if (opts.find(L"obj") != opts.end()) account = opts[L"obj"];
        if (opts.find(L"DisplayName") != opts.end()) displayName = opts[L"DisplayName"];
        //Execute the config changes
        ChangeServiceConfig(sh, dwServiceType, dwStartType, dwErrorControl,
            binPath.empty() ? nullptr : binPath.c_str(),
            loadOrderGroup.empty() ? nullptr : loadOrderGroup.c_str(),
            nullptr, dependencies.empty() ? nullptr : dependencies.c_str(),
            account.empty() ? nullptr : account.c_str(),
            nullptr, displayName.empty() ? nullptr : displayName.c_str());
        wprintf(L"[SC] ChangeServiceConfig SUCCESS\n");
        if (opts.find(L"description") != opts.end()) {
            SERVICE_DESCRIPTION sd;
            sd.lpDescription = (wchar_t*)opts[L"description"].c_str();
            ChangeServiceConfig2(sh, SERVICE_CONFIG_DESCRIPTION, &sd);
            wprintf(L"[SC] ChangeServiceConfig2 DESCRIPTION SUCCESS\n");
        }
        CloseServiceHandle(sh);
        CloseServiceHandle(sch);
    }


    // Failure Command – configure service failure actions (restart, run command, reboot)
    else if (cmd == L"failure") {
        //Store the command argument
        const wchar_t* serviceName = argv[2];
        //Open Service Manager
        SC_HANDLE sch = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        //Open Service with config permissions
        SC_HANDLE sh = OpenService(sch, serviceName, SERVICE_CHANGE_CONFIG);
        //Parse through options and store the key
        std::map<std::wstring, std::wstring> opts;
        for (int i = 3; i < argc; i++) {
            wchar_t* eq = wcschr(argv[i], L'=');
            if (eq && i + 1 < argc) {
                std::wstring key(argv[i], eq - argv[i]);
                opts[key] = argv[i + 1];
                i++;
            }
        }
        //Store the failure actions configuration
        SERVICE_FAILURE_ACTIONS fa = { 0 };
        //Store the individual actions extracted
        std::vector<SC_ACTION> actions;
        //Reset Code
        DWORD resetPeriod = INFINITE;
        std::wstring rebootMsg, command;
        if (opts.find(L"reset") != opts.end()) resetPeriod = _wtoi(opts[L"reset"].c_str());

        //Actions Code
        if (opts.find(L"actions") != opts.end()) {
            //Holding string with actions specification
            const wchar_t* a = opts[L"actions"].c_str();
            //Create vector to hold the tokens after being split to match API format
            std::vector<std::wstring> parts;
            wchar_t* ctx = nullptr;
            wchar_t* token = wcstok_s(const_cast<wchar_t*>(a), L"/", &ctx);
            //Loop through till no more tokens are found
            while (token) { parts.push_back(token); token = wcstok_s(nullptr, L"/", &ctx); }
            if (parts.size() % 2 == 0) {
                for (size_t j = 0; j < parts.size(); j += 2) {
                    SC_ACTION act;
                    if (parts[j] == L"restart") act.Type = SC_ACTION_RESTART;
                    else if (parts[j] == L"run") act.Type = SC_ACTION_RUN_COMMAND;
                    else if (parts[j] == L"reboot") act.Type = SC_ACTION_REBOOT;
                    else act.Type = SC_ACTION_NONE;
                    act.Delay = _wtoi(parts[j + 1].c_str());
                    actions.push_back(act);
                }
            }
        }

        //Reboot Code
        if (opts.find(L"reboot") != opts.end()) rebootMsg = opts[L"reboot"]; 

        //Command Code
        if (opts.find(L"command") != opts.end()) command = opts[L"command"];

        //Fill the failure actions structre
        fa.dwResetPeriod = resetPeriod;
        fa.lpRebootMsg = rebootMsg.empty() ? nullptr : (wchar_t*)rebootMsg.c_str();
        fa.lpCommand = command.empty() ? nullptr : (wchar_t*)command.c_str();
        fa.cActions = (DWORD)actions.size();
        fa.lpsaActions = actions.data();
        //Apply configuration and print success
        ChangeServiceConfig2(sh, SERVICE_CONFIG_FAILURE_ACTIONS, &fa);
        wprintf(L"ChangeServiceConfig FAILURE_ACTIONS Successful!\n");
        CloseServiceHandle(sh);
        CloseServiceHandle(sch);
    }


    // Qfailure Command – query service failure actions
    else if (cmd == L"qfailure") {
        //Open Service Manager
        SC_HANDLE sch = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        //Open Service with config permissions
        SC_HANDLE sh = OpenService(sch, argv[2], SERVICE_QUERY_CONFIG);
        DWORD bytesNeeded = 0;
        QueryServiceConfig2(sh, SERVICE_CONFIG_FAILURE_ACTIONS, nullptr, 0, &bytesNeeded);
        std::vector<BYTE> buffer(bytesNeeded);
        LPSERVICE_FAILURE_ACTIONS fa = (LPSERVICE_FAILURE_ACTIONS)buffer.data();
        //Retrieve failure actions and print them
        QueryServiceConfig2(sh, SERVICE_CONFIG_FAILURE_ACTIONS, buffer.data(), bytesNeeded, &bytesNeeded);
        wprintf(L"\n QueryServiceConfig FAILURE_ACTIONS Successful!\n\n");
        wprintf(L"SERVICE_NAME: %ls\n", argv[2]);
        wprintf(L"   RESET_PERIOD: %lu\n", fa->dwResetPeriod);
        wprintf(L"   REBOOT_MESSAGE: %ls\n", fa->lpRebootMsg ? fa->lpRebootMsg : L"");
        wprintf(L"   COMMAND_LINE: %ls\n", fa->lpCommand ? fa->lpCommand : L"");
        wprintf(L"   ACTIONS: ");
        //Loop for each action to print
        for (DWORD i = 0; i < fa->cActions; i++)
            wprintf(L"%d/%lu ", fa->lpsaActions[i].Type, fa->lpsaActions[i].Delay);
        wprintf(L"\n");
        CloseServiceHandle(sh);
        CloseServiceHandle(sch);
    }
}