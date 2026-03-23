# DFOR 740 Midterm – Windows Service Control Emulator

This tool emulates the functionality of the Windows `sc.exe` utility, allowing you to manage Windows services from the command line. It is written in C++ and uses the Windows Service Control Manager API.

**Executable name:** `DFOR 740.exe` (note the space – when running from the command line, enclose the name in quotes or rename the file to remove the space.)

## Compilation

### Using Visual Studio (IDE)
1. Create a new **Console Application** project.
2. Replace the content of the main `.cpp` file with the provided source code.
3. In **Project Properties → Linker → Input → Additional Dependencies**, add `advapi32.lib` (if not already present).
4. Build the solution (F7). The executable will be created in the `Debug` or `Release` folder.

### Running the Code Using Command Line (Developer Command Prompt)
1. Open a **Developer Command Prompt for Visual Studio** (as Administrator if you want to test immediately).
2. Navigate to the folder containing the executable file (e.g., `DFOR 740.exe`).
3. If you run the tool without any arguments, or with `help` or `?`, a help screen is displayed.

---

## Commands

### `query` – Display service status
- Without `service`: lists all service names.
- With `service`: shows detailed status (type, state, exit codes, process ID, display name).

### `qc` – Query configuration
- Displays the service’s configuration: type, start type, error control, binary path, dependencies, account name, etc.

### `create` – Install a new service
**Required:** `binPath=` – path to the executable.

**Optional options:**
- `type= own|share` – service type (own process or shared process). Default: `own`.
- `start= auto|demand|disabled` – start type. Default: `demand`.
- `error= normal|severe|critical|ignore` – error control. Default: `normal`.
- `DisplayName=` – friendly name.
- `depend=` – dependencies (list of services separated by `/`).
- `obj=` – account name (e.g., `NT AUTHORITY\LocalService`).
- `password=` – password for the account (if needed).


### `qdescription` – Query description
- Shows the service description (if any).

### `start` – Start a service

### `stop` – Stop a service

### `delete` – Remove a service

### `config` – Modify service configuration
### `create` – Install a new service
**Required:** `binPath=` – path to the executable.

**Optional options:**
- `type= own|share` – service type (own process or shared process). Default: `own`.
- `start= auto|demand|disabled` – start type. Default: `demand`.
- `error= normal|severe|critical|ignore` – error control. Default: `normal`.
- `DisplayName=` – friendly name.
- `depend=` – dependencies (list of services separated by `/`).
- `obj=` – account name
- `password=` – password for the account (if needed).
- 'description=' - set or change the description

### `failure` – Set failure actions
- `reset=` – time (in seconds) after which the failure counter is reset to zero. Use `INFINITE` for never resetting.
- `actions=` – a list of actions and delays (in milliseconds), separated by `/`.  
  **Action types:** `restart`, `run`, `reboot`.
- `reboot=` – message to display if the action is `reboot`.
- `command=` – command line to run if the action is `run`

### `qfailure` – Query failure actions
- Displays the current failure recovery settings.

## Help Screen
- Running the tool without arguments or with `help` or `?` displays a summary of all commands and their syntax.

---

## Important Notes

- All commands that modify the service configuration must be run **as Administrator**.
- The service binary path must be accessible and correctly quoted if it contains spaces.
- Failure actions are only effective if the service is configured to restart on failure (see `sc failure` documentation).

---

## Detection and Logging

When the tool is executed, it generates events that can be captured by Sysmon and the Windows Event Log.  
- **Sysmon** records process creation (Event ID 1) and registry changes under `HKLM\System\CurrentControlSet\Services`.  
- The **System log** (source: Service Control Manager) records service installations (7045), configuration changes (7040), state changes (7036), and deletions (7042).  

These logs can be used to detect the use of a custom service‑management tool and to correlate its actions with process telemetry.

---

## License
This tool is provided for educational purposes only. Use responsibly.
