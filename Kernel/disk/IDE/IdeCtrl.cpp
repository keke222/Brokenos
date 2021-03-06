#include <disk/IDE/IdeCtrl.h>
#include <video/Screen.h>
#include <pci/pci.h>

#include <disk/IDE/IdeDrive.h>

Vector<IdeCtrl*> *IdeCtrl::IdeList = nullptr;

IdeCtrl::IdeCtrl(u8 bus, u8 device, u8 function)
    : _bus(bus), _device(device), _function(function)
{
    _primaryPorts[0] = 0x1F0;
    _primaryPorts[1] = 0x3F6;

    _secundaryPorts[0] = 0x170;
    _secundaryPorts[1] = 0x376;

    checkPorts();

    _drives[0] = new IdeDrive(_primaryPorts[0], _primaryPorts[1], Master);
    _drives[1] = new IdeDrive(_primaryPorts[0], _primaryPorts[1], Slave);

    _drives[2] = new IdeDrive(_secundaryPorts[0], _secundaryPorts[1], Master);
    _drives[3] = new IdeDrive(_secundaryPorts[0], _secundaryPorts[1], Slave);


    for(int i = 0; i < 4; ++i)
        _connetedDevice[i] = _drives[i]->isConnected();
}

IdeDrive& IdeCtrl::getDrive(BusRole bus, DriveRole drive)
{
    unsigned int index = (bus == SecundaryBus) ? 1 : 0; // 0 if primary, else 1
    index *= 2;
    index += (drive == Slave);

    return *_drives[index];
}

void IdeCtrl::displayModelNames()
{
    for(int i = 0; i < 4; i++)
        if(_connetedDevice[i])
            Screen::getScreen().printk("IDE Device : %s\n", _drives[i]->getModelName());
}

void IdeCtrl::displayTree()
{
    sScreen.println("Ide controller: PCI(%u, %u, %u)", _bus, _device, _function);
    sScreen.putcar(0xB3);
    sScreen.putcar(0x0A);

    sScreen.putcar(0xC3);
    sScreen.printk(" Primary master : %s\n", _drives[0]->getModelName());

    sScreen.putcar(0xC3);
    sScreen.printk(" Primary slave : %s\n", _drives[1]->getModelName());

    sScreen.putcar(0xC3);
    sScreen.printk(" Secundary master : %s\n", _drives[2]->getModelName());

    sScreen.putcar(0xC0);
    sScreen.printk(" Secundary slave : %s\n", _drives[3]->getModelName());
}

void IdeCtrl::checkPorts()
{
    u8 progIf = pciConfigReadByte(_bus, _device, _function, 0x09);

    if(!(progIf & 0x1))
        return;
    else
    {
        _primaryPorts[0] = 0xFFFC & pciConfigReadWord(_bus, _device, _function, 0x10);
        _primaryPorts[1] = 0xFFFC & pciConfigReadByte(_bus, _device, _function, 0x14);

        _secundaryPorts[0] = 0xFFFC & pciConfigReadWord(_bus, _device, _function, 0x18);
        _secundaryPorts[1] = 0xFFFC & pciConfigReadByte(_bus, _device, _function, 0x1C);
    }
}