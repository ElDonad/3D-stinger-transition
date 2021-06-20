*A little experimental plugin aimed at generating 3D transitions. Currently only one transition is available and working, although the plugin is a bit quirky, it will definitively be improved in the future ! (also this is my first OBS related project, so lot of stuff to learn and overcome...)*


**How to use :**

* Install the plugin from the github release page
* Create your own transition video using the provided blender file (you will need to create a video with transparency. The common method is to use ffmpeg with the command [CODE]ffmpeg -i %04d.png out.webm[/CODE], although for perfomance reasons I prefer to use the .mov format along with the qtrle encoder, you might want to experiment on that)
* Create the transition in obs :
    * adjust the transition time (3240 ms for the currently provided transition)
    * Set the video file path in the transition's properties
    * Tune the delay between the video and the transition in the transition's properties


**Current quirks :** 

* The transition isn't able to set its own duration. The current transition lasts exactly 3240ms, you will need to set the transition duration to just that.
* On some less powerful devices, the video and the scene transition might get a bit out of sync. This can be tuned in the parameters of the source, with a default value meaning that the transition will start 20ms after the video begins. You might want to tune that to your liking.

**TODO :**

* Add more translations
* Allow automatic setup of the transition duration
* Generalization to any user defined transform : My current thoughts on that is that I will need to create a small blender addon which will be able to extract the transformation of an object and output it in an exploitable file from the OBS plugin's side. But I've not given many thought yet ; I'm welcome on suggestions !
* Add the ability to display simultaneously the two sources
* Refactor the effect files, which are currently terrible
* Allow the user to set its own effect files

**DEMO :**
https://youtu.be/ylafBweCZFQ
