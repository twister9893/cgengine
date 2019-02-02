Object.prototype.forEach = function(func)
{
    func(this);
}
String.prototype.capitalized = function()
{
    return this.charAt(0).toUpperCase() + this.slice(1);
}
String.prototype.decapitalized = function()
{
    return this.charAt(0).toLowerCase() + this.slice(1);
}
String.prototype.contains = function(substr)
{
    return (this.indexOf(substr) !== -1)
}




function Model(name, sequences, maps, enums, structs, topics)
{
    this.name = name;
    this.NAME = name.toUpperCase();
    this.sequences = sequences;
    this.maps = maps;
    this.enums = enums;
    this.structs = structs;
    this.topics = topics;
}

function Sequence(name, type)
{
    this.Name = name;
    this.type = type;
}

function Map_(name, keyType, valueType)
{
    this.Name = name;
    this.keyType = keyType;
    this.valueType = valueType;
}

function Enum(name, type, enumerators)
{
    this.Name = name;
    this.type = type;
    this.enumerators = enumerators;
}

function Enumerator(name, value)
{
    this.Name = name;
    this.value = value;
}




function Structure(name, members)
{
    this.name = name.decapitalized();
    this.Name = name;
    this.NAME = name.toUpperCase();
    this.members = members;

    for (var i = 0; i < this.members.length; ++i) {
        this.members[i].struct = { name : this.name, 
                                   Name : this.Name, 
                                   NAME : this.NAME };
    }
}

Structure.prototype.includeList = function()
{
    var list = '';
    this.members.forEach( function(m) 
    {
        var istr = m.type.includeString();
        if (istr && !list.contains(istr)) {
            list += istr + '\n';
        }
    } );
    return list;
}

Structure.prototype.forwardList = function()
{
    var list = '';
    this.members.forEach( function(m) 
    {
        var fstr = m.type.forwardString();
        if (fstr & !list.contains(fstr)) {
            list += fstr + '\n';
        }
    } );
    return list;
}



function TypeWrapper(name)
{
    this.name = name;
}
TypeWrapper.prototype.wrapped = function(value)
{
    switch (this.name)
    {
        case 'Angle':        return 'sysmath::units::Angle::deg('+value+')';
        case 'Distance':     return 'sysmath::units::Distance::km('+value+')';
        case 'Velocity':     return 'sysmath::units::Velocity::mps('+value+')';
        case 'TimeInterval': return 'sysmath::units::TimeInterval::msec('+value+')';
    }
    return value;
}
TypeWrapper.prototype.unwrapped = function(value)
{
    switch (this.name)
    {
        case 'Angle':        return value+'.deg()';
        case 'Distance':     return value+'.km()';
        case 'Velocity':     return value+'.mps()';
        case 'TimeInterval': return value+'.msec()';
    }
    return value;
}



function Member(name, type, definition)
{
    this.name = name;
    this.Name = name.capitalized();
    this.type = type;
    this.definition = definition;

    if (this.property('type') !== '') {
        this.type.wrapper = new TypeWrapper(this.property('type'));
    }
}

Member.prototype.property = function(name)
{
    if (this.definition === undefined) {
        return '';
    }
    var rxStr = '\\$\\{${name}=(.*)\\}'.replace("${name}", name);
    var rx = new RegExp(rxStr,'g');
    var rxRes = rx.exec(this.definition);
    return (rxRes === null ? '' : rxRes[1]);
}

Member.prototype.setterDeclaration = function()
{
    if (this.type.primitive) {
        return 'void set'+this.Name+'('+this.type.name+' '+this.name+');';
    } else {
        return 'void set'+this.Name+'(const '+this.type.name+' &'+this.name+');';
    }
    return '';
}

Member.prototype.getterDeclaration = function()
{
    return this.type.name+' '+this.name+'() const;';
}

Member.prototype.rgetterDeclaration = function()
{
    return this.type.name+' & '+this.name+'();';
}

Member.prototype.hasDeclaration = function()
{
    return 'bool has'+this.Name+'() const;';
}




function Type(name, category, namespace)
{
    switch (name)
    {
        case 'Boolean' :  this.name = 'bool';           this.primitive = true; this.defaultValue = 'false'; break;
        case 'Byte' :     this.name = 'quint8';         this.primitive = true; this.defaultValue = '0'; break;
        case 'Int16' :    this.name = 'qint16';         this.primitive = true; this.defaultValue = '0'; break;
        case 'UInt16' :   this.name = 'quint16';        this.primitive = true; this.defaultValue = '0'; break;
        case 'Int32' :    this.name = 'qint32';         this.primitive = true; this.defaultValue = '0'; break;
        case 'UInt32' :   this.name = 'quint32';        this.primitive = true; this.defaultValue = '0'; break;
        case 'Int64' :    this.name = 'qint64';         this.primitive = true; this.defaultValue = '0'; break;
        case 'UInt64' :   this.name = 'quint64';        this.primitive = true; this.defaultValue = '0'; break;
        case 'Float32' :  this.name = 'float';          this.primitive = true; this.defaultValue = '0.0f'; break;
        case 'Float64' :  this.name = 'double';         this.primitive = true; this.defaultValue = '0.0'; break;
        case 'Float128' : this.name = 'long double';    this.primitive = true; this.defaultValue = '0.0'; break;
        case 'Char8' :    this.name = 'char';           this.primitive = true; this.defaultValue = '0'; break;
        case 'Char32' :   this.name = 'wchar_t';        this.primitive = true; this.defaultValue = '0'; break;
        case 'String8' :  this.name = 'std::string';    this.primitive = false; break;
        case 'String32' : this.name = 'std::wstring';   this.primitive = false; break;
        default :         this.name = name;             this.primitive = false; break;
    }

    switch (category)
    {
        case 'sequence' :    this.reference = true; break;
        case 'map' :         this.reference = true; break;
        case 'enumeration' : this.reference = false; this.primitive = true; break;
        case 'string' :      this.reference = false; break;
        case 'structure' :   this.reference = true; break;
        default :            this.reference = false; break;
    }

    this.category = category;
    this.namespace = namespace;
}

Type.prototype.includeString = function()
{
    switch (this.category)
    {
        case 'sequence':
        case 'map':
        case 'enumeration':
            // return '#include <'+this.namespace+'/'+this.namespace+'.h>';
            return '#include <'+this.namespace+'/'+this.namespace+'-types.h>';
        case 'string':
            return (this.name === 'std::string' ? '#include <string>' : '#include <wstring>');
        case 'structure':
            return '#include <'+this.namespace+'/'+this.name.toLowerCase()+'-struct.h>';
        default:
            return (this.wrapper !== undefined ? '#include <sysmath/units.h>' : '')
    }
}

Type.prototype.forwardString = function()
{
    switch (this.category)
    {
        case 'sequence':
        case 'map':
        case 'structure':
            return 'namespace '+this.namespace+' { class '+this.name+'; }';
        case 'enumeration':
            return 'namespace '+this.namespace+' { enum class '+this.name+'; }';
        case 'string':
            return (this.name === 'std::string' ? 'namespace std { class string; }' : 'namespace std { class wstring; }');
    }
    return '';
}



function Topic(struct, keys)
{
    this.struct = struct;
    this.keys = keys;
}

Topic.prototype.singleton = function()
{
    return ( this.keys.length === 0 );
}



function typeCategory(type)
{
    if (type == 'String8' || type == 'String32') {
        return 'string';
    }

    var cat = '';

    if (modelDom.module.constructedtypes.sequence !== undefined)
    {
        modelDom.module.constructedtypes.sequence.forEach( function(seq) {
            if (type === seq.name) {
                cat = 'sequence';
            }
        } );
    }

    if (modelDom.module.constructedtypes.map !== undefined)
    {
        modelDom.module.constructedtypes.map.forEach( function(map) {
            if (type === map.name) {
                cat = 'map';
            }
        } );
    }

    if (modelDom.module.constructedtypes.enumeration !== undefined)
    {
        modelDom.module.constructedtypes.enumeration.forEach( function(enm) {
            if (type === enm.name) {
                cat = 'enumeration';
            }
        } );
    }

    if (modelDom.module.constructedtypes.structure !== undefined)
    {
        modelDom.module.constructedtypes.structure.forEach( function(struc) {
            if (type === struc.name) {
                cat = 'structure';
            }
        } );
    }
    return cat;
}

function findStruct(Name, structs)
{
    for (var i = 0; i < structs.length; ++i)
    {
        if (structs[i].Name == Name) {
            return structs[i];
        }
    }
}




function readSequences(typesDom, namespace)
{
    var array = [];
    if (typesDom.sequence === undefined) {
        return array;
    }

    typesDom.sequence.forEach( function(seq) {
        array.push(new Sequence(seq.name, 
                                new Type(seq.seqType,
                                         typeCategory(seq.seqType),
                                         namespace)));
    } );
    return array;
}

function readMaps(typesDom, namespace)
{
    var array = [];
    if (typesDom.map === undefined) {
        return array;
    }

    typesDom.map.forEach( function(map) {
        array.push(new Map_(map.name, 
                            new Type(map.typeMapKey,
                                     typeCategory(map.typeMapKey),
                                     namespace),
                            new Type(map.typeElement,
                                     typeCategory(map.typeElement),
                                     namespace)));
    } );
    return array;
}

function readEnumerators(enumDom)
{
    var array = [];
    if (enumDom.enumerator === undefined) {
        return array;
    }

    enumDom.enumerator.forEach( function(e) 
    {
        array.push( new Enumerator(e.name,
                                   e.value) );
    } );
    return array;
}

function readEnums(typesDom, namespace)
{
    var array = [];
    if (typesDom.enumeration === undefined) {
        return array;
    }

    typesDom.enumeration.forEach( function(enm) {
        array.push(new Enum(enm.name, 
                            new Type(enm.type,
                                    typeCategory(enm.type),
                                    namespace),
                            readEnumerators(enm)));
    } );
    return array;
}

function readStructs(typesDom, namespace)
{
    var array = [];
    if (typesDom.structure === undefined) {
        return array;
    }

    typesDom.structure.forEach( function(struc) {
        array.push(new Structure(struc.name, 
                                 readMembers(struc, namespace)));
    } );
    return array;
}

function readMembers(structDom, namespace)
{
    var array = [];
    if (structDom.memberAgregate === undefined) {
        return array;
    }

    structDom.memberAgregate.forEach( function(memb) 
    {
        array.push( new Member(memb.name, 
                               new Type(memb.type,
                                        typeCategory(memb.type),
                                        namespace),
                               memb.definition) );
    } );
    return array;
}

function readTopics(moduleDom, namespace)
{
    var array = [];
    if (moduleDom.topicDataType === undefined) {
        return array;
    }

    var structs = readStructs(moduleDom.constructedtypes, 
                              moduleDom.namespace);

    moduleDom.topicDataType.forEach( function(topic)
    {
        var keyArray = [];
        if (topic.keys !== null) {
            topic.keys.key.forEach( function(key) {
                keyArray.push(key);
            } );
        }

        array.push( new Topic(findStruct(topic.structName,
                                         structs),
                              keyArray) );
    } );
    return array;
}

function readModel()
{
    var obj = new Model(modelDom.module.namespace, 
                        readSequences(modelDom.module.constructedtypes, 
                                      modelDom.module.namespace),
                        readMaps(modelDom.module.constructedtypes, 
                                 modelDom.module.namespace),
                        readEnums(modelDom.module.constructedtypes, 
                                  modelDom.module.namespace),
                        readStructs(modelDom.module.constructedtypes, 
                                    modelDom.module.namespace),
                        readTopics(modelDom.module, 
                                   modelDom.module.namespace));
    return obj;
}

var model = readModel();

var headComment = 
'/*******************************************\n'+
' *              AUTOGENERATED              *\n'+
' *******************************************/\n';
