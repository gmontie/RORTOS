# RORTOS
Register Oriented Real-Time Operating System


My Control Systems are usually implemented as a Registered Oriented Control System. Over the years slow devices would cause my implementations to miss there mark. So it was iether generate un-maintainable code, which was ungly and basically a one off, or figure out a way to background the offending hardware interface. The latter in my view requires the CPU to be pre-empted. Think of it as the CPU as a service. Basically whenever there is a slow interface which is waiting for hardware it can now be put in it's own little task. 

I also find that isolating things down to just the individual numbers which actually implement the control systems simplifies the implementation. It is in the vent of thinking that I have come up with and have worked on a generic Register Oriented type of control system where. 
