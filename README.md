# rawinput-debug
Win32 console application for demonstrating an issue with raw input and system idle state

# Background

A few months ago, I thought I ought to try Microsoft Flight Simulator 2020. [The facsimile of our planet](https://www.youtube.com/watch?v=0w7q1ZFfsxs) that Asobo had created with photogrammetry and machine learning seemed like a good place to relax, *["in these trying times."](https://www.powerthesaurus.org/in_these_trying_times/synonyms)* I plugged in my trusty Logitech Freedom 2.4 wireless joystick and took to the skies.

![image](https://user-images.githubusercontent.com/1832518/101274624-befac300-376d-11eb-88be-f68bf37ebddf.png) ![image](https://user-images.githubusercontent.com/1832518/101274626-c1f5b380-376d-11eb-903e-8450feefc55f.png)


After spending a few hours flying around my alma mater and my childhood home, it was time to call it a day. I have my machine configured to turn off the displays after a few minutes of inactivity, and I quickly realized it wasn't doing that any longer.

This isn't something I'm altogether unfamiliar with. Sometimes it is a browser tab open with a sneaky video trying to play in the background, sometimes it is an issue with a web application, and sometimes it is a system task deciding it is too important to allow the machine to get anywhere near Standby/Sleep while it completes. On Windows, applications can request this privilege - of keeping the machine awake - from the operating system's [Kernel-Mode Power Manager](https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/windows-kernel-mode-power-manager). This is useful as you wouldn't want your machine going to sleep or turning off the display while you are watching a movie, playing a game, or copying a file. You can see such requests by opening an [elevated command prompt](https://superuser.com/questions/968214/open-cmd-as-admin-with-windowsr-shortcut) and running [`powercfg /requests`](https://docs.microsoft.com/en-us/windows-hardware/design/device-experiences/powercfg-command-line-options).

![image](https://user-images.githubusercontent.com/1832518/101274616-adb1b680-376d-11eb-8964-11840af6e751.png)

Nothing.

I had an idea, though. I had previously contributed to a project called [procrastitracker](https://github.com/aardappel/procrastitracker), a fantastic little time tracking application for Windows, and the feature I implemented was [Xinput](https://docs.microsoft.com/en-us/windows/win32/xinput/getting-started-with-xinput) activity detection. You see, I used the application to track how much time I spent playing games, and it thought my machine was idle when I was using the Xbox controller, so my change was just to have the application detect XInput as user activity. One thing I had noticed is that Windows does use controller inputs - not just mouse & keyboard inputs - to determine that the machine is currently in use, and prevent the display from sleeping. I also noticed that one of my controllers' analog inputs would actually drift around quite a bit, so my naïve implementation without deadzones was not going to work, or procrastitracker would think I was using my controller while I slept. Deadzones solved the issue with procrastitracker, and the controller never kept Windows awake, so that was good. Was my old Logitech joystick to blame? It is easily 15 years old by now. I unplugged the receiver and within minutes my display went to sleep. *Mystery solved!* 

Not so fast.

# Enter NVIDIA

I'd made a few upgrades to my computer in the months since, which included getting a better joystick, and a throttle, and some flight rudder pedals. Windows still wasn't letting the display sleep with them plugged in, but I decided I should look into the issue more. Looking at the USB Game Controllers control panel with the new joystick plugged in, I noticed absolutely no drift. No analog instability. No spurious inputs. It certainly wasn't the device's activity keeping the machine awake, I was sure of that now.

![image](https://user-images.githubusercontent.com/1832518/101274635-d9cd3780-376d-11eb-8e1d-e381bf7f4dc5.png)

This is a screenshot, but it doesn't look different in motion. Nothing moves.

I decided to do what's worked so well in the past - I asked Google. 

![image](https://user-images.githubusercontent.com/1832518/101274645-ee113480-376d-11eb-8ee3-dc1a1b0826ae.png)

And Google knew.  People had tracked the issue down to the NVIDIA's GeForce Experience overlay, sometimes called ShadowPlay (or NVIDIA Share). It's a piece of software that allows you to use the NVIDIA graphics cards' [NVENC encoder](https://en.wikipedia.org/wiki/Nvidia_NVENC) to capture compressed video in real time. People use it to share videos of moments in video games, and it's very handy because NVENC is *good*. Compressing high-resolution high-framerate video in real time on the CPU while maintaining quality would be quite a difficult task, especially for a machine already tasked with running a video game, and NVENC produces quality output without much additional load on the machine by leveraging fixed-function encoding hardware in the GPU. It's cool stuff, so I didn't want to just get rid of it.

So the issue is such: If you have a joystick plugged in, and the GeForce Experience overlay enabled, your display will not sleep. If you unplug the joystick, the display sleeps. If you disable the overlay, the display sleeps. You can have one or the other - but not both. 

People hadn't just tracked the issue down - people tracked it down 3 years ago!  

![image](https://user-images.githubusercontent.com/1832518/101274648-f23d5200-376d-11eb-9258-3a8e3ce62ef7.png)

I couldn't really believe the issue had gone unresolved for so long. So I reported a bug.  

![image](https://user-images.githubusercontent.com/1832518/101274650-f5384280-376d-11eb-96e6-4d024a193c30.png)

I'm sure they'll figure it out, but I wanted to have a crack at it. When you enable the overlay, many processes start up - all of the NVIDIA ones at the top.

![image](https://user-images.githubusercontent.com/1832518/101274654-f8cbc980-376d-11eb-9a25-25d179af547a.png)

Each one loads many modules:

![image](https://user-images.githubusercontent.com/1832518/101274692-30d30c80-376e-11eb-93a2-038fbb4478d7.png)

My initial theory was that the overlay was perhaps probing the controllers for input, and translating those events into [Windows messages](https://docs.microsoft.com/en-us/windows/win32/winmsg/using-messages-and-message-queues). If it was injecting messages into one of its processes, maybe as keyboard events, perhaps some default event handling routine was resetting the system idle state. I had no evidence, but what I knew was that it wasn't Windows acting alone. The NVIDIA software was causing the issue, but the overlay doesn't react to joystick input anyway, so I somehow doubted that it was an accidental side-effect of intentional joystick-handling code. Abusing Win32 [isn't unusual among GPU makers](https://www.techpowerup.com/274967/psa-amds-graphics-driver-will-eat-one-cpu-core-when-no-radeon-installed), so I expected it would be something weird. I should note at this point I am not an expert in Win32. On the other hand, I do own [Raymond Chen's book](https://www.amazon.ca/Old-New-Thing-Development-Throughout/dp/0321440307) and I've read it, too. [The Old New Thing](https://devblogs.microsoft.com/oldnewthing/)  is a great blog. I'm still a bit lost here, though.

I digress. First I needed a way to identify when the issue was happening without waiting for the display to sleep, so I quickly wrote a simple application that dumps the output of [`GetLastInputInfo`](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getlastinputinfo). I didn't expect this function to be authoritative on the system idle state - you want to get [`SYSTEM_POWER_INFORMATION from CallNtPowerInformation`](https://docs.microsoft.com/en-us/windows/win32/api/powerbase/nf-powerbase-callntpowerinformation) for that - but it proved to be effective.

![image](https://user-images.githubusercontent.com/1832518/101274704-43e5dc80-376e-11eb-9564-55582626063c.png)

I attached to `NVIDIA Share.exe` in [x64dbg](https://x64dbg.com) and started looking for things related to input.

![image](https://user-images.githubusercontent.com/1832518/101274705-4e07db00-376e-11eb-857f-ab66b4e1dd0a.png)

I knew Xinput wasn't causing the issue - that's only for Xbox Controllers and those who emulate them, plus I knew procrastitracker was in the background polling Xinput all the time anyway and didn't cause this. What I did notice was that even with the process suspended in the debugger (and my little application monitoring idle state), I could see it was still getting reset. There are many processes it spawned, I thought, so I went through and suspended them one by one (killing them one by one doesn't work - they restart immediately). The idle state kept resetting. This was a huge clue - it means the application wasn't running any code to do this. It wasn't injecting messages or anything of the sort. **It has to be something it does on initialization.**

I wanted to see how NVIDIA Share initializes, in the debugger, but it's complicated. You can't start it directly, it needs to be started by nvcontainer.exe. It starts three copies of it, each with different parameters. They probably communicate with each other, as well, so their environment would have to be carefully managed to bring them up manually. Not insurmountable by any means, but there were other things to try. I thought it would be neat if I could attach & break in x64dbg as soon as the process starts, and [some tips](https://github.com/x64dbg/x64dbg/issues/847) pointed me to [WinDbg](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools)'s `gflags.exe` utility.

![image](https://user-images.githubusercontent.com/1832518/101274711-56f8ac80-376e-11eb-88ea-6dfbacba6015.png)

Theoretically you can use it to throw a key in the registry that tells Windows to execute a particular 'image' (executable) with a debugger when it's encountered. I wasn't able to get this to work - maybe because the process is spawned by nvcontainer, or maybe I just hadn't done it correctly.

Luckily, we have [Ghidra](https://ghidra-sre.org/). I did the same silly thing that I did in the debugger, I loaded up the most obvious executable (`NVIDIA Share.exe`) and asked the most obvious question. 

# "Y'all got any input stuff 'round here?"

![image](https://user-images.githubusercontent.com/1832518/101274720-62e46e80-376e-11eb-9112-cf4ff35464e6.png)

This was immediately promising! But first, I had to do some reading. Raw input isn't something that I'm familiar with. Back in the good old days, there was DirectInput. DirectInput let you do force feedback, DirctInput let you have tons of buttons and axes, and at least on Windows it made using games controllers generally a smoother experience than it had been in the past, where games needed to support your particular controller (or your controller's drivers needed to emulate another, more popular controller). After DirectInput came Xinput, and Xinput is very much built around the Xbox Controller. You don't get any more buttons or axes than an Xbox Controller can have. You can't connect more controllers than an Xbox would be able to connect. It "just works", but it's not the kind of API that supports uses like this: 

![image](https://user-images.githubusercontent.com/1832518/101274723-6aa41300-376e-11eb-9c12-86ddf57fc82e.png)

The photo is not mine.

Now that heavyweight API is raw input. Anything that conforms to the [Human Interface Device](https://en.wikipedia.org/wiki/Human_interface_device) standard will have its events passed through, and your role as an application developer is to support the [HID usage pages](https://usb.org/sites/default/files/hut1_21_0.pdf) that you deem appropriate. I especially like that in the middle of the Simulations Control page (`0x02`) is the usage ID for Magic Carpet Simulation (`0x0B`). Standards committees think of everything.

![image](https://user-images.githubusercontent.com/1832518/101274727-742d7b00-376e-11eb-8b51-5f87e61ce64b.png)

So, what is NVIDIA Share doing with raw input? [`RegisterRawInputDevices`](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerrawinputdevices).

![image](https://user-images.githubusercontent.com/1832518/101274732-7c85b600-376e-11eb-9347-0c0be1f76d47.png)

Don't worry, I can clean it up a bit:

![image](https://user-images.githubusercontent.com/1832518/101274739-86a7b480-376e-11eb-8519-6e49d6da4422.png)

It's registering its window handle to receive raw events from the keyboard at all times (regardless of which window is in the foreground). Keyboard, bummer. Not joystick. But it gave me an idea. What if I expand my little application to request raw input as well? What about DirectInput? Can I replicate the issue without NVIDIA's software? I spent one night and one day implementing various inputs methods, relearning Win32, and learning DirectInput ... and [COM ](https://en.wikipedia.org/wiki/Component_Object_Model)... again.

![image](https://user-images.githubusercontent.com/1832518/101274743-90c9b300-376e-11eb-8b72-af022d238557.png)

I was able to replicate the issue. 

### Enabling raw input either as a system-wide input sink (`dwFlags = RIDEV_INPUTSINK 0x100`) or only on foreground focus (`default, dwFlags = 0x0`) causes devices to flood the `HWND`'s message queue with [`WM_INPUT`](https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-input)s, and prevents the system from becoming idle. My suggestion to Microsoft would be to make this clearer in the documentation, and to have an application requesting raw input show up in `powercfg /requests` and in [`WPA`](https://docs.microsoft.com/en-us/windows-hardware/test/wpt/windows-performance-analyzer). The application I wrote to demonstrate the issue is available in this repo: <https://github.com/nuzayets/rawinput-debug/>

But NVIDIA Share wasn't asking for raw input from the joystick. 

Not directly, anyhow. NVIDIA Share is partially built upon [CEF, Chromium Embedded Framework](https://en.wikipedia.org/wiki/Chromium_Embedded_Framework). Why be happy with only wrapping your head around esoteric desktop development when you can throw frustrating web development into the mix? The more the merrier, I say. We didn't need that RAM anyway.

NVIDIA Share loads the Chromium Embedded Framework as an >100MB module called `libcef.dll`. This took Ghidra a bit of time to analyze, but I found the interesting bit. 

![image](https://user-images.githubusercontent.com/1832518/101274756-a2ab5600-376e-11eb-9466-9be27b4b175a.png)

They request raw input as part of their gamepad driver, which makes sense. They call that `FUN_1842af9b4` to set up its parameters in all cases. Here is that function: 

![image](https://user-images.githubusercontent.com/1832518/101274760-a939cd80-376e-11eb-94ea-2f0043abd2c0.png)

If you don't speak decompliation-ese, here's a rough translation:

![image](https://user-images.githubusercontent.com/1832518/101274763-b1920880-376e-11eb-9183-ffaece3a115b.png)

Luckily there wasn't any code patching to do. The values for the usage IDs live in the `.rdata` section of the executable (that `DAT_1861e16e8` in Ghidra's decompilation).

The file is `C:\Program Files\NVIDIA Corporation\NVIDIA GeForce Experience\libcef.dll` and with my version of GeForce Experience (3.20.5.70), the offending byte was at `0x61e0ae8`. Changing the `0x04` to a `0x06` means that instead of trying to get raw input from joysticks, they get it from the keyboard instead. I'm still not sure why the NVIDIA overlay was asking for raw joystick input from Chromium.

![image](https://user-images.githubusercontent.com/1832518/101274768-b9ea4380-376e-11eb-90fd-fd73b339d04a.png)

I spent two days on this, and it ended up being one byte in the end. At least now my computer can sleep.

![image](https://user-images.githubusercontent.com/1832518/101274776-cb335000-376e-11eb-9b96-bb4261227a43.png)
