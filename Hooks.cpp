#include "main.h"

void InitializeInterfaces() {
    pSurface = GetInterface<ISurface>("./bin/osx64/vguimatsurface.dylib", "VGUI_Surface");
    pPanel = GetInterface<IPanel>("./bin/osx64/vgui2.dylib", "VGUI_Panel");
    pCvar = GetInterface<ICvar>("./bin/osx64/materialsystem.dylib", "VEngineCvar");
    pClient = GetInterface<IBaseClientDLL>("./csgo/bin/osx64/client.dylib", "VClient");
    pEngine = GetInterface<IEngineClient>("./bin/osx64/engine.dylib", "VEngineClient");
    pEntList = GetInterface<IClientEntityList>("./csgo/bin/osx64/client.dylib", "VClientEntityList");
    pOverlay = GetInterface<IVDebugOverlay>("./bin/osx64/engine.dylib", "VDebugOverlay");
    pEngineTrace = GetInterface<IEngineTrace>("./bin/osx64/engine.dylib", "EngineTraceClient");
    pModelInfo = GetInterface<IVModelInfo>("./bin/osx64/engine.dylib", "VModelInfoClient");
    pInputSystem = GetInterface<IInputSystem>("./bin/osx64/inputsystem.dylib", "InputSystemVersion");
    pModelRender = GetInterface<IVModelRender>("./bin/osx64/engine.dylib", "VEngineModel");
    pMatSystem = GetInterface<IMaterialSystem>("./bin/osx64/materialsystem.dylib", "VMaterialSystem");
    pPrediction = GetInterface<IPrediction>("./csgo/bin/osx64/client.dylib", "VClientPrediction");
    pGameMovement = GetInterface<IGameMovement>("./csgo/bin/osx64/client.dylib", "GameMovement");
    pPhysics = GetInterface<IPhysicsSurfaceProps>("./bin/osx64/vphysics.dylib", "VPhysicsSurfaceProps");
    //pMoveHelper = **(IMoveHelper***)(GetLibraryAddress("/client.dylib") + 0x511DA08); // Search for "Player.Swim" in "client.dylib".
}

void ProtectAddr(void* addr, int prot)
{
    long pagesize = sysconf(_SC_PAGESIZE);
    void* address = (void *)((long)(uintptr_t)addr & ~(pagesize - 1));
    
    mprotect(address, sizeof(address), prot);
}
void InitializeVMTs() {
    painthook = new VMT(pPanel);
    
    uintptr_t findClientMode = CPatternScanner::Instance()->GetPointer("client.dylib", (unsigned char*)"\xE8\x00\x00\x00\x00\x48\xC7\xC3\x00\x00\x00\x00\x4C\x8D\x3D\x00\x00\x00\x00\x0F\x1F\x44\x00", "x????xxx????xxx????xxxx", 0xF) + 0x4;
    
    uintptr_t findGlobalVars = CPatternScanner::Instance()->GetPointer("client.dylib", (unsigned char*)"\x48\x8D\x05\x00\x00\x00\x00\x48\x8B\x00\xF3\x0F\x10\x00\x00\xF3\x0F\x11\x83\x00\x00\x00\x00", "xxx????xxxxxx??xxxx????", 0x3) + 0x4;
    
    uintptr_t booladdr = CPatternScanner::Instance()->GetProcedure("engine.dylib", (unsigned char*)"\x41\xB5\x00\x84\xC0\x74\x11", "xx?xxxx", 0x1) + 0x2;
        //booladdr = GetAbsoluteAddress(booladdr, 2, 1);
        bSendPacket = reinterpret_cast<bool*>(booladdr);
        ProtectAddr(bSendPacket, PROT_READ | PROT_WRITE | PROT_EXEC);
        //vm_protect(mach_task_self(), (vm_address_t)bSendPacket, 1, 0, VM_PROT_ALL);
    
    
    pClientMode = reinterpret_cast<IClientMode*>(findClientMode);
    pGlobals = *reinterpret_cast<CGlobalVarsBase**>(findGlobalVars);
    
    createmovehook = new VMT(pClientMode);
    clientvmt = new VMT(pClient);
    modelhook = new VMT(pModelRender);
    predhook = new VMT(pPrediction);
}

void InitializeHooks() {
    painthook->HookVM((void*)hkPaintTraverse, 42);
    painthook->ApplyVMT();
    
    createmovehook->HookVM((void*)hkCreateMove, 25);
    createmovehook->HookVM((void*)hkOverrideView, 19);
    createmovehook->ApplyVMT();
    
    clientvmt->HookVM((void*)hkKeyEvent, 20);
    clientvmt->HookVM((void*)hkFrameStage, 36);
    clientvmt->ApplyVMT();
    
    modelhook->HookVM((void*)hkDrawModelExecute, 21);
    modelhook->ApplyVMT();
    
    predhook->HookVM((void*)hkRunCommand, 20);
    predhook->ApplyVMT();
}

void UpdateResolver() {
    OldProxy_X = (RecvVarProxyFn)NetVarManager::HookProp("DT_CSPlayer", "m_angEyeAngles[0]", FixPitch);
    OldProxy_Y = (RecvVarProxyFn)NetVarManager::HookProp("DT_CSPlayer", "m_angEyeAngles[1]", FixYaw);
   // OldProxy_Z = (RecvVarProxyFn)NetVarManager::HookProp("DT_BasePlayer", "m_vecViewOffset[2]", ViewOffsetZProxy);
   // OldProxy_Velocity = (RecvVarProxyFn)NetVarManager::HookProp("DT_BasePlayer", "m_vecBaseVelocity", BaseVelocityProxy);
}
