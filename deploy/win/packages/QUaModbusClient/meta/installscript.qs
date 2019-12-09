function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        // define shortcuts
        allshortcuts = [{ appname : "/bin/QUaModbusClient.exe", shortcutname : "QUaModbusClient"}];
		// create shortcuts
        for (var i = allshortcuts.length - 1; i >= 0; i--) {
            // start menu 
            component.addOperation("CreateShortcut", 
                "@TargetDir@" + allshortcuts[i].appname, 
                "@StartMenuDir@/" + allshortcuts[i].shortcutname + ".lnk",
                "workingDirectory=@TargetDir@",
                "iconId=2");
            // desktop
             component.addOperation("CreateShortcut", 
                "@TargetDir@" + allshortcuts[i].appname, 
                "@DesktopDir@/" + allshortcuts[i].shortcutname + ".lnk",
                "workingDirectory=@TargetDir@",
                "iconId=2");
        }
        // uninstaller shortcut
        component.addOperation("CreateShortcut", 
            "@TargetDir@/maintenancetool.exe", 
            "@StartMenuDir@/Uninstall.lnk",
            "workingDirectory=@TargetDir@",
            "iconId=2");
        // install architecture dependencies
        // (start installer with -v to see debug output)
        console.log("CPU Architecture: " +  systemInfo.currentCpuArchitecture);
        if ( systemInfo.currentCpuArchitecture === "i386") {
            // execute compiler runtime
            component.addElevatedOperation("Execute", "{0,3010}", "@TargetDir@/redist/VC2017_redist.x86.exe","/norestart", "/q");     
        }
        if ( systemInfo.currentCpuArchitecture === "x86_64") {
            // execute compiler runtime
            component.addElevatedOperation("Execute", "{0,3010}", "@TargetDir@/redist/VC2017_redist.x64.exe","/norestart", "/q");            
        }
    }
    else
    {

    }
}