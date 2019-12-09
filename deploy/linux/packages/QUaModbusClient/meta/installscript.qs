function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") 
    {
        
    }
    else
    {
    	// run bootstrap script
        component.addOperation("Execute", "@TargetDir@/LinuxBootstrap.sh")
    }
}