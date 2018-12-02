var STRUCT = 'TARGET';
var Struct = 'Target';
var struct = 'target';
var model = 'sys';

var members = [
                { name : 'heading', Name : 'Heading',  type : 'Angle',         primitive : true,  reference : false, includeDirective : '#include <sys-math/units.h>' },
                { name : 'speed',   Name : 'Speed',    type : 'Velocity',      primitive : true,  reference : false, includeDirective : '#include <sys-math/units.h>' },
                { name : 'pos',     Name : 'Pos',      type : 'CoordWrapper',  primitive : false, reference : true,  includeDirective : '#include <sys/coord-wrapper.h>' }
              ];

var structs = [
                { NAME : 'ACTIVETASK', name : 'activetask' },
                { NAME : 'LAYOUT', name : 'layout' },
                { NAME : 'THEME', name : 'theme' },
                { NAME : 'PALETTE', name : 'palette' }
              ];

var __repeats__ = structs.length;