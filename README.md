# The Esrille New Keyboard - NISSE
Copyright 2013-2016 Esrille Inc.

This product includes the design and the firmware of the Esrille New Keyboard
being developed at Esrille Inc. (http://www.esrille.com/keyboard/).

See the file NOTICE for copying permission.

# Usage

## Linux (Ubuntu Xenixal 16.04)

### Compile your custom keyboard firmware (.hex file) with MPLAB X IDE

You need to download 2 things from http://www.microchip.com:

- MPLAB X IDE (http://www.microchip.com/mplab/mplab-x-ide) (version 3.51 or higher)
- XC8 Compiler v1.34 (http://www.microchip.com/development-tools/downloads-archive)

. You do not need the Microchip Libraries for Applications (MLA) (Version: v2013-12-20) because this repo already comes with a copy of it.

The Linux versions of these two programs are installed by just untarring (if necessary), setting the execute bit (`chmod +x`) on the install script, and running it with `sudo`.
They are basically standalone binary installs and install into the `/opt` folder.

For Ubuntu Xenial 16.04, you also need to have 32-bit libraries installed as the MPLAB/XC8 programs are both 32-bit applications.

```
sudo apt-get install libc6:i386 libx11-6:i386 libxext6:i386 libstdc++6:i386 libexpat1:i386
```

You'll also need libusb-1.0:

```
sudo apt-get install libusb-1.0
```

.

You should now be able to run the MPLAB X IDE, and then you can open an existing MPLAB project by selecting this folder:

```
new-keyboard/firmware/third_party/mla_v2013_12_20/apps/usb/device/hid_keyboard/firmware/MPLAB.X
```

You should now be able to click on Build to build the project.
The output is a `.hex` file (should be at `new-keyboard/firmware/third_party/mla_v2013_12_20/apps/usb/device/hid_keyboard/firmware/MPLAB.X/dist/Esrille_New_Keyboard/production/MPLAB.X.production.hex`) to feed into the USB Bootloader program.

### Compile the USB Bootloader program

To run the USB Bootloader program, you need to get QtCreator.

```
sudo apt-get install qtcreator qt5-default
```

Then you can run QtCreator and open up an existing project, and point to

```
new-keyboard/firmware/third_party/mla_v2013_12_20/apps/usb/device/bootloaders/utilities/qt5_src/HIDBootloader.pro
```

to open up the "HID Bootloader" project.

Now do `CTRL-B` or click on `Build -> Build Project "HID Bootloader"` in the menu to compile it.
Then run the Bootloader with `CTRL-R` (or the green arrow in the GUI).
You are now ready to overwrite the existing firmware on the NISSE with the `.hex` file from earlier.

### Load your custom firmware (.hex file) with USB Bootloader program

Hold down the default ESCAPE key on the NISSE; while still holding it down, connect it to your Linux machine.
The red LED on the NISSE should start blinking (ready for firmware upgrade).
In the USB Bootloader, click on the open folder icon and point it to the `new-keyboard/firmware/third_party/mla_v2013_12_20/apps/usb/device/hid_keyboard/firmware/MPLAB.X/dist/Esrille_New_Keyboard/production/MPLAB.X.production.hex` file from earlier.
Then click on the other icon to overwrite the firmware.
You should get a message like this:

```
Starting Erase/Program/Verify Sequence.
Do not unplug device or disconnect power until the operation is fully complete.

Erasing Device... (no status update until complete, may take several seconds)
Erase Complete (1.362s)

Writing Device...
Write Complete (1.79s)

Verifying Device...
Verify Complete (1.408s)

Erase/Program/Verify sequence completed successfully.
You may now unplug or reset the device.
Resetting...
```

That's it!
Rinse and repeat as necessary.
