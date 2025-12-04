# Rabenhaus

Check out some gameplay here: https://youtu.be/aLy6DQPm9QE

Now that the winter season is upon us, the holidays are coming up and I have more free time in general, I have decided to give another try to a field of IT I tried to enter many times before to not much avail: **Operating Systems**. This project started off pretty rocky as well, but after switching to Limine it was a much better experience than anything I have tried before :D The name **Rabenhaus** is a literal German translation (because everything sounds cool in German :P) of *Hollóháza*, (literally "the house of (the) raven") which is a small village in the Zemplén region of Hungary where I used to go sledding when I was a kid :)

What I ended up with is not really an OS in the traditional sense, more so a bootable game: You take the role of a sledder sledding down a hill, and you have to avoid hitting trees. But be warned, for the more trees you pass the faster the game will get! <br>
If you do die, you get greeted by a totally-not-copied-from-Windows Blue Screen of Death (quite literally cuz you die if you hit a tree :P), where pressing any key restarts your computer. The OS was tested and is confirmed booting and playable on real x64 hardware! (More specifically I tested it on a HP Pavilion 15 laptop).

<br>

<img width="1018" height="764" alt="Képernyőkép 2025-12-03 232754" src="https://github.com/user-attachments/assets/b971a629-9d95-4222-94df-b394e3838f74" />

<br>

Now I can't say that this project is a fully fledged complete OS or even just a usable hobby OS; It's a little tech demo game, really. However I got much further than ever before, I learned a good amount about lower-level programming and the inner workings of OSes, and although I can't say that I didn't need to look up Stack Overflow, the OSDev Wiki (really great resource btw!) or (blasphemy) ChatGPT a few times, I am still proud of what I have achieved and I think it came out great! 

I would like to one day try my hand at making an *actual* operating system in the traditional sense (filesystems, programs etc), but if (and when) I try a similar project again I might learn Rust first and then do [Philipp Oppermann's tutorial](https://os.phil-opp.com/) to learn the basics better; It seems like a really well put together tutorial and I can't wait to try it out - maybe for Boot YSWS?


## How to build
1. Download the code and unzip
2. Set up Limine in the folder via [This tutorial](https://wiki.osdev.org/Limine_Bare_Bones)
3. Run `build.sh`
4. If you want to rebuild, run `make clean` first.


## How to run
1. Either build the OS or download the newest release
2. Burn the .iso to a USB or insert it into a VM
3. Enjoy! This requires no installation (I have no idea how you could even do that 😭)


## System requirements
CPU: Anything that's 64-bit <br>
Aaand that's pretty much it; **Rabenhaus 1 - Win11 0** 😎
