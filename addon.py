bl_info = {
    "name": "Export to OBS",
    "blender": (2, 80, 0),
    "category": "Object",
}

import bpy
import json
from math import radians, sin, cos

def e_to_q(roll, pitch, yaw):

        qx = sin(roll/2) * cos(pitch/2) * cos(yaw/2) - cos(roll/2) * sin(pitch/2) * sin(yaw/2)
        qy = cos(roll/2) * sin(pitch/2) * cos(yaw/2) + sin(roll/2) * cos(pitch/2) * sin(yaw/2)
        qz = cos(roll/2) * cos(pitch/2) * sin(yaw/2) - sin(roll/2) * sin(pitch/2) * cos(yaw/2)
        qw = cos(roll/2) * cos(pitch/2) * cos(yaw/2) + sin(roll/2) * sin(pitch/2) * sin(yaw/2)

        return [qw, qx, qy, qz]

class OBJECT_OT_exporttransformobs(bpy.types.Operator):
    """My Object Moving Script"""      # Use this as a tooltip for menu items and buttons.
    bl_idname = "object.exporttransformobs"        # Unique identifier for buttons and menu items to reference.
    bl_label = "Export transform for OBS"         # Display name in the interface.
    bl_options = {"REGISTER"}
    
    subframe_resolution: bpy.props.FloatProperty(name="Subframe resolution", default=2.0, max=10)
    swap_frame: bpy.props.IntProperty(name="Swap frame", default=100)
    filepath: bpy.props.StringProperty(subtype="FILE_PATH")
    
    @classmethod
    def poll(self, context):
        return context.object is not None
    
    def execute(self, context):        # execute() is called when running the operator.
        data = []
        print(context.object.name)
        scene = context.scene
       
        
        scene.frame_set(scene.frame_start)
        
        frame = (scene.frame_start,1)
        
        while frame[0] != scene.frame_end:
            loc = context.object.location
            rot = context.object.rotation_euler
            rot = e_to_q(rot[0], rot[1], rot[2])
            scale = context.object.scale
            if frame[1] >= self.subframe_resolution:
                frame = (frame[0] + 1, 1)
            else:
                frame = (frame[0], frame[1] + 1)
            scene.frame_set(frame[0], subframe=1.0 / frame[1])
            
            data.append({"position": {"x": loc.x/10.0, "y": -loc.y/10.0, "z": loc.z/10.0},
            "rotation": {"x": rot[1], "y": rot[2], "z": rot[3], "w": rot[0]},
            "scale": {"x": scale[0], "y": scale[1], "z": scale[2]}})
        file = open(self.filepath, 'w')
        jsonData = {"swap_time": self.swap_frame / scene.frame_end, "data_type": "interpolation", "data": {"frames": data, "resolution": self.subframe_resolution}}  
        file.write(json.dumps(jsonData, indent=4))
        file.close()
        
        print("DONE")
        return {'FINISHED'}            # Lets Blender know the operator finished successfully.
    
    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

def menu_func(self, context):
    self.layout.operator(OBJECT_OT_exporttransformobs.bl_idname)
    

def register():
    bpy.utils.register_class(OBJECT_OT_exporttransformobs)
    bpy.types.VIEW3D_MT_object.append(menu_func)

def unregister():
    bpy.utils.unregister_class(OBJECT_OT_exporttransformobs)
    bpy.types.VIEW3D_MT_object.remove(menu_func)
    
if __name__ == "__main__":
    register()