from . import types
from . import dep
from . import RuntimeModel
from . import Metadata
module_list = []
available_module_names_list = []


class Property(object):
    def __init__(self, name, description, namespace='', visible=False, readonly=False, define_name=None, long_description=None):
        '''
        :param name: Name of the property (string)
        :param type: PropertyType describing the type of property
        :param visible: Boolean, whether the property is visible in the UI or not
        '''
        self.name = name
        self.type = type
        self.dependencies = []
        self.generation = []
        self.visible = visible
        self.readonly = readonly
        self.category=''
        self.subcategory=''
        self.namespace = namespace
        self.id = '.'.join((str(namespace).upper(), self.name))
        self.label = description
        self.description = long_description
        self.defaultvalue = ''
        self.transient = False
        self.parent = None
        self.mode = None
        self.define_name = define_name if define_name else name
        self.is_advanced = False
        self.allowedconflicts = []

    def set_parent_module(self, mod):
        self.parent = mod

    def set_namespace(self, namespace):
        '''
        Sets the namespace on a property
        :param namespace:
        :return:
        '''
        self.namespace = namespace
        self.id = '.'.join((str(namespace).upper(), self.name, 'PROP'))

    def set_visibility(self, visible):
        self.visible = visible

    def set_readonly(self, readonly):
        self.readonly = readonly

    def add_dependency(self, dependency):
        self.dependencies.append(dependency)

    def get_dependencies(self):
        return self.dependencies

    def generateXML(self):
        '''
        Generate the Studio XML for this property
        :return: etree.Element containing the Studio XML describing the property
        '''
        print("Not able to gen XML from base property!")
        print(self.name)
        print(self.type)
        return None


class StringProperty(Property):
    '''
    Property which can take on a string value
    '''
    def __init__(self, name, description, namespace='', visible=False, readonly=False, define_name=None, long_description=None):
        Property.__init__(self, name, description, namespace=namespace, visible=visible, readonly=readonly, define_name=define_name, long_description=long_description)
        self.id = '.'.join((str(namespace).upper(), self.name, 'STRING'))

    def set_namespace(self, namespace):
        self.id = '.'.join((str(namespace).upper(), self.name, 'STRING'))


class ArrayProperty(Property):
    '''
    Property which can take on an array value
    '''
    def __init__(self, name, description, namespace='', visible=False, readonly=False, define_name=None, long_description=None):
        Property.__init__(self, name, description, namespace=namespace, visible=visible, readonly=readonly, define_name=define_name, long_description=long_description)
        self.id = '.'.join((str(namespace).upper(), self.name, 'ARRAY'))

    def set_namespace(self, namespace):
        self.id = '.'.join((str(namespace).upper(), self.name, 'ARRAY'))


class IntegerProperty(Property):
    '''
    Property which can take on integer values
    '''
    def __init__(self, name, description, min, max, default, namespace='', visible=False, readonly=False, define_name=None, long_description=None):
        Property.__init__(self, name, description, namespace=namespace, visible=visible, readonly=readonly, define_name=define_name, long_description=long_description)
        self.min = int(min)
        self.max = int(max)
        self.defaultvalue = int(default)
        self.id = '.'.join((str(namespace).upper(), self.name, 'INT'))

    def set_namespace(self, namespace):
        self.id = '.'.join((str(namespace).upper(), self.name, 'INT'))


class Enum:
    '''
    Container class for an item inside of an EnumProperty
    '''
    def __init__(self, value, index, define_value=None, visible=True):
        self.value = value
        self.visible = visible
        self.index = index

        if define_value is not None:
            self.define_value = define_value
        else:
            self.define_value = value


class EnumProperty(Property):
    '''
    Property allowing a selection from a list of options
    '''
    def __init__(self, name, description, namespace='', visible=False, readonly=False, define_name=None, long_description=None):
        Property.__init__(self, name, description, namespace=namespace, visible=visible, readonly=readonly, define_name=define_name, long_description=long_description)
        self.values = {}
        self.id = '.'.join((str(namespace).upper(), self.name, 'ENUM'))

    def set_namespace(self, namespace):
        self.id = '.'.join((str(namespace).upper(), self.name, 'ENUM'))

    def add_enum(self, value, define_value=None, visible=True):
        '''
        Add an option to the selection list
        :param value: String value for the option (visible in UI)
        :param visible: Whether or not this option will be visible in the UI
        :param define_value: Name which will be generated as #def value
        :return: None
        '''
        self.values[len(self.values.keys())] = Enum(value, len(self.values.keys()), define_value=define_value, visible=visible)


class ModeProperty(EnumProperty):
    def __init__(self, name, description, namespace='', visible=False, readonly=False, define_name=None, long_description=None):
        Property.__init__(self, name, description, namespace=namespace, visible=visible, readonly=readonly, define_name=define_name, long_description=long_description)
        self.values = {}
        self.id = '.'.join((str(namespace).upper(), self.name, 'ENUM'))


class BoolProperty(EnumProperty):
    '''
    Property allowing you to select a binary setting
    '''
    def __init__(self, name, description, namespace='', visible=False, readonly=False, define_name=None, long_description=None):
        EnumProperty.__init__(self, name, description, namespace=namespace, visible=visible, readonly=readonly, define_name=define_name, long_description=long_description)
        self.id = '.'.join((str(namespace).upper(), self.name, 'BOOL'))

        self.add_enum('False', define_value="0")
        self.add_enum('True', define_value="1")

    def set_namespace(self, namespace):
        self.id = '.'.join((str(namespace).upper(), self.name, 'BOOL'))

    def set_default_to_false(self):
        self.defaultvalue = 'False'

    def set_default_to_true(self):
        self.defaultvalue = 'True'


class ModuleProperty(EnumProperty):
    '''
    Property allowing you to select a peripheral available on the current chip
    '''
    def __init__(self, name, description, namespace='', visible=False, readonly=False, define_name=None, long_description=None):
        EnumProperty.__init__(self, name, description, namespace=namespace, visible=visible, readonly=readonly, define_name=define_name, long_description=long_description)
        self.id = '.'.join((str(namespace).upper(), self.name, 'MOD'))
        self.allowedModules = []
        self.inherit_options = False
        self.define_value_prefix = ''
        self.owned_mode = None
        self.define_name_postfix = ''

    def set_namespace(self, namespace):
        self.id = '.'.join((str(namespace).upper(), self.name, 'MOD'))

    def add_allowed_module(self, module_namespace):
        '''
        Adds a module 'namespace' to the allowed modules for this property
        :param module_namespace: regular expression for which modules can be selected by this property
        :return: None
        '''
        self.allowedModules.append(module_namespace)

    def mask_with_module_list(self, module_name_list):
        '''
        Updates the list of allowed modules for this property by comparing a list with the property's allowed modules.
        :param module_name_list: list of module names available on this part
        :return:
        '''
        self.values = {}
        self.add_enum('None')
        for mod_name in module_name_list:
            for allowed_mod in self.allowedModules:
                if mod_name.rstrip('0123456789') == allowed_mod:
                    define_value = mod_name
                    self.add_enum(mod_name, define_value=define_value)


class PinProperty(EnumProperty):
    '''
    Property allowing you to select any GPIO pin available
    '''
    def __init__(self, name, description, namespace='', visible=False, readonly=False, define_name=None, disabled_label=None, long_description=None):
        EnumProperty.__init__(self, name, description, namespace=namespace, visible=visible, readonly=readonly, define_name=define_name, long_description=long_description)
        self.referenced_module = None
        self.referenced_route = None
        self.em4 = False
        self.id = '.'.join((str(namespace).upper(), self.name, 'PIN'))
        if not disabled_label:
            disabled_label = 'Disabled'
        self.disabled_label = disabled_label
        self.add_enum(disabled_label, define_value="Disabled")

    def set_namespace(self, namespace):
        self.id = '.'.join((str(namespace).upper(), self.name, 'PIN'))

    def mask_with_pin_list(self, pin_list):
        '''
        Updates the available enum values with the values from pin_list
        :param pin_list: list of pin names available on the part
        :return: None
        '''
        self.values={}
        self.add_enum(self.disabled_label, define_value="Disabled")
        for pin in pin_list:
            self.add_enum(pin)

    def set_reference_route(self, route):
        self.referenced_route = route

    def set_reference_module(self, module):
        self.referenced_module = module

    def set_reference(self, module, route):
        self.referenced_module = module
        self.referenced_route = route


class PRSChannelProperty(EnumProperty):
    """
    Property allowing you to select PRS channel available from the PRS module
    """
    def __init__(self, name, description, channel_count, custom_name="", namespace='', visible=False, readonly=False, define_name=None, long_description=None):
        EnumProperty.__init__(self, name, description, namespace=namespace, visible=visible, readonly=readonly, define_name=define_name, long_description=long_description)
        self.add_enum("Disabled")
        self.channel_count = channel_count
        self.custom_name = custom_name
        for i in range(channel_count):
            self.add_enum("CH" + str(i), define_value=str(i))

class StudioModule(object):
    """docstring for StudioModule"""
    def __init__(self, basename, modules):
        super(StudioModule, self).__init__()
        self.basename = basename
        self.modules = {}
        for m in modules:
            # Allow both lists of frameworks and single framework to be specified
            if isinstance(m.frameworks,list):
                for framework in m.frameworks:
                    self.modules[framework.value] = m.name
            else:
                self.modules[m.frameworks.value] = m.name

    def getModuleId(self, framework):
        print ("%s: %s" % (self.basename, self.modules.keys()))
        if framework not in self.modules:
            return None
        return "%s.%s" % (self.basename, self.modules[framework])

    def __str__(self):
        return self.basename


class Module(object):
    '''
    Class for describing a HALConfig module or device peripheral. A module is basically a collection of properties.
    '''
    def __init__(self, name, core=False, visible=False, namespace=None):
        # Name is a required argument
        self.name = name
        self.displayname = name
        # namespace defaults to module base (i.e. module without the instance number)
        if namespace:
            self.namespace = namespace
        else:
            self.namespace = name.rstrip('0123456789')
        # No description by default
        self.description = ""
        # Core signifies a module contributed by the die
        self.core = core
        # Visible controls whether the module shows up in the UI
        self.visible = visible
        # List of properties on this module
        self.properties = []
        # Category is the category where to put the module on the UI. Default for core is 'Core'.
        self.category = '  Peripherals' if core else " HAL"
        # Define generated with the module being active (selected) or not
        self.enabled_define = None
        # Compatibility of module
        self.compatibility = dep.Dependency()
        # Association with an on-chip peripheral
        self.peripheral = None
        # Studio module specifier
        self.studio_module = None
        # By default, module has no custom name property
        self.has_custom_name = False

        self.model = None
        self.family = None

        # Contribute 'standard' properties for every module, allowing SDKs to take control in a hwconf doc
        # (even though they shouldn't)
        inuse = BoolProperty('usedbysdk', 'SDK is taking control over this module', visible=False)
        self.add_property(inuse)
        hidden = BoolProperty('hiddenbysdk', 'SDK is hiding this module', visible=False)
        self.add_property(hidden)
        hidden = BoolProperty('showadvanced', 'Show advanced options', visible=False)
        self.add_property(hidden)

        owner = StringProperty('owner', 'Owned by', visible=True, readonly=True)
        owner.transient = True
        self.add_property(owner)

        if self.core and (self.namespace != self.name):
            # Add custom name property if this is a parameterized core module (e.g. USARTn, TIMERn...)
            customname = StringProperty('customname', 'Custom name', visible=True, readonly=False)
            self.add_property(customname)
            self.has_custom_name = True

        existing = list(filter(lambda mod: mod.name == self.name, module_list))
        if existing:
            pass
            # print("Module " + self.name + " already in global ExporterModel.module_list")
        else:
            module_list.append(self)

    def __str__(self):
        if self.studio_module:
            return str(self.studio_module)
        return "none"

    def add_property(self, prop):
        '''
        Add a property to this module
        :type prop: Property
        :param prop: property to add
        :return: None
        '''
        # Regular list append for now
        # TODO: change to property merge on properties with same ID
        prop.set_namespace(self.namespace)
        prop.set_parent_module(self)
        self.properties.append(prop)

    def load_halconfig_model(self, family=None):
        '''
        Load a HAL config model
        :param model:   a HAL config model
        :param family:  a halconfig_dependency Family object describing for which family this module is loaded or
        str containing family name
        :return: None
        '''
        if not family:
            raise ValueError("Family is not set")
        if isinstance(family, str):
            self.family = dep.Family(family_str=family)
        else:
            self.family = family
        self.family.available_mods = available_module_names_list
        if hasattr(self.model, 'compatibility'):
            self.compatibility = self.model.compatibility

        if hasattr(self.model, "peripheral"):
            self.peripheral = self.model.peripheral

        if hasattr(self.model, "category"):
            self.category = self.model.category

        if hasattr(self.model, "displayname"):
            self.displayname = self.model.displayname

        if hasattr(self.model, "description"):
            self.description = self.model.description

        if hasattr(self.model, "studio_module"):
            self.studio_module = StudioModule(self.model.studio_module["basename"], \
                                              self.model.studio_module["modules"])

        if hasattr(self.model, 'modes'):
            mode_prop = ModeProperty(self.model.modes["define"], "mode", visible=True)
            for val in self.model.modes["values"]:
                if isinstance(val, types.EnumValue):
                    if val.dependency:
                        if val.dependency.applies_to(family=self.family):
                            mode_prop.add_enum(val.display_name, define_value=val.define_value)
                    else:
                        mode_prop.add_enum(val.display_name, define_value=val.define_value)
                else:
                    mode_prop.add_enum(val)
            self.add_property(mode_prop)

        if hasattr(self.model, 'enable'):
            self.enabled_define = self.model.enable["define"]

        for prop, options in self.model.options.items():
            current_opt_set = None

            # If one property has several option elements, iterate to find which option element has the correct dependency
            if isinstance(options, list):
                for opt in options:
                    # Skip documentation option
                    if opt.get("documentation"):
                        continue
                    if opt.get("dependency"):
                        if opt.get("dependency").applies_to_family(self.family):
                            if opt.get("dependency").applies_to_module(self.name):
                                current_opt_set = opt
                                break
                        
            else:
                if options.get("dependency"):
                    if options.get("dependency").applies_to_family(self.family):
                        if options.get("dependency").applies_to_module(self.name):
                            current_opt_set = options
                else:
                    current_opt_set = options
            if current_opt_set is not None:
                self._load_halconfig_property(prop, current_opt_set, self.family, self.model)

        self.post_load()

    def _load_halconfig_property(self, prop, opts, family, model):
        """
        :param prop:    a HAL config property
        :param opts:    dictionary containing a set of options for current prop
        :param family:  a halconfig_dependency Family object describing for which family this module is loaded
        :return: None
        """
        prop_obj = None
        extra_properties = []
        if opts['type'] == 'enable':
            self.enabled_define = prop
        elif opts['type'] == 'boolean':
            prop_obj = BoolProperty(prop, opts['description'], visible=True)
        elif opts['type'] == 'integer':
            prop_obj = IntegerProperty(prop, opts['description'], opts['min'], opts['max'], 0, visible=True)
        elif isinstance(opts['type'], str) and 'int' in opts['type'] and '_t' in opts['type']:
            prop_obj = IntegerProperty(prop, opts['description'], opts['min'], opts['max'], 0, visible=True)
        elif opts['type'] == 'string':
            prop_obj = StringProperty(prop, opts['description'], visible=True)
        elif opts['type'] == 'array':
            prop_obj = ArrayProperty(prop, opts['description'], visible=True)
        elif opts['type'] == 'enum':

            if opts['values']:
                prop_obj = EnumProperty(prop, opts['description'], visible=True)
                for val in opts['values']:
                    if isinstance(val, types.EnumValue):
                        if val.dependency:
                            if val.dependency.applies_to_family(family=family):
                                prop_obj.add_enum(val.display_name, define_value=val.define_value)
                        else:
                            prop_obj.add_enum(val.display_name, define_value=val.define_value)
                    else:
                        prop_obj.add_enum(val)

        elif isinstance(opts['type'], types.Pin):
            prop_obj = PinProperty(prop, opts['description'], visible=True, disabled_label=opts['type'].disabled_label)
            if opts['type'].signal:
                # Pin is connected to a PORTIO signal
                prop_obj.set_reference(self.name, opts['type'].signal)
            if opts['type'].em4:
                prop_obj.em4 = True

        elif isinstance(opts['type'], types.Peripheral):
            prop_obj = ModuleProperty(prop, opts['description'], visible=True)
            for filter in opts['type'].filter:
                prop_obj.add_allowed_module(filter)
            prop_obj.inherit_options = opts['type'].inherit_options
            prop_obj.define_value_prefix = opts['type'].define_value_prefix
            prop_obj.define_name_postfix = opts['type'].define_name_postfix
            if hasattr(opts['type'], 'mode'):
                prop_obj.owned_mode = opts['type'].mode
        elif isinstance(opts['type'], types.PinArray):
            prop_obj = IntegerProperty(opts['type'].count_define, opts['description'], opts['type'].min, opts['type'].max, opts['type'].default, visible=True)

            init_string = ""
            for i in range(opts['type'].min, opts['type'].max):
                visible = True if i < opts['type'].default else False
                item_property = PinProperty(opts['type'].item_define.replace("%n", str(i)), opts['type'].item_description.replace("%n", str(i)), visible=visible)
                if opts.get('allowedconflicts') is not None:
                    item_property.allowedconflicts = opts['allowedconflicts']
                if visible:
                    init_string += ("{{ {0}, {1} }}, ".format(opts['type'].item_port_define.replace("%n", str(i)),
                                                           opts['type'].item_pin_define.replace("%n", str(i))))
                extra_properties.append(item_property)

            if init_string:
                # Strip last comma space from default value
                init_string = init_string[:-2]

            init_property = ArrayProperty(opts['type'].init_define, "{} init".format(prop), visible=False)
            init_property.defaultvalue = init_string
            init_property.transient = True
            extra_properties.append(init_property)
        elif isinstance(opts['type'], types.PRSChannelLocation):
            prs_chan_count = Metadata.get_prs_chan_with_gpio_count(family.get_name())
            prop_obj = PRSChannelProperty(opts['type'].define, opts['description'], prs_chan_count,
                                          custom_name=opts['type'].custom_name, visible=True)
            extra_properties.append(StringProperty("prs_disabled_chn_{}_pin".format(opts['type'].custom_name if opts['type'].custom_name else ""), "PRS channel output pin",
                                                   visible=True, readonly=True, long_description="No PRS channel selected"))
            if dep.Dependency(platform=dep.Platform.SERIES0).applies_to_family(family):
                # Make PRS dropdown readonly on Series 0, since changing it will affect unrelated modules that
                # also use PRS. Users will have to use PORTIO view to select PRS location.
                readonly = True
            else:
                readonly = False
            for i in range(prs_chan_count):
                item_property = PinProperty(opts['type'].name + str(i),
                                            opts['type'].output_description.replace("%n", str(i)),
                                            visible=False,
                                            readonly=readonly,
                                            define_name=opts['type'].name)
                item_property.set_reference("PRS", "CH" + str(i))
                extra_properties.append(item_property)

        elif opts['type'] == 'route_loc':
            # This is a 180nm-style route location. We ignore it, since the route location is given by the pin.
            pass
        else:
            print("ERROR: unknown property type {} in HAL config model for {}".format(opts['type'], model.name))

        if prop_obj is not None:
            if opts.get('mode') is not None:
                prop_obj.mode = opts['mode']

            # Hiding properties that don't belong to the default mode
            mode_prop = next((prop for prop in self.get_properties() if isinstance(prop, ModeProperty)), None)
            if mode_prop:
                if hasattr(prop_obj, 'mode'):
                    if isinstance(prop_obj.mode, list):
                        prop_obj.set_visibility(True if mode_prop.values[0].define_value in prop_obj.mode else False)
                    elif prop_obj.mode:
                        prop_obj.set_visibility(True if prop_obj.mode == mode_prop.values[0].define_value else False)

            # If _model specifically states visibility, this overrides hiding by default mode
            if opts.get("visible") is not None:
                prop_obj.set_visibility(opts['visible'])
            if opts.get("advanced", False):
                # Hide advanced properties by default
                prop_obj.is_advanced = opts.get("advanced", False)
                prop_obj.set_visibility(False)
            if opts.get("readonly") is not None:
                prop_obj.set_readonly(opts['readonly'])
            if opts.get('defaultValue') is not None:
                prop_obj.defaultvalue = opts['defaultValue']
            if opts.get('longdescription') is not None:
                prop_obj.description = opts['longdescription']
            elif opts.get("default") is not None:
                prop_obj.defaultvalue = opts['default']
            if opts.get('subcategory') is not None:
                prop_obj.subcategory = opts['subcategory']
            if opts.get('allowedconflicts') is not None:
                prop_obj.allowedconflicts = opts['allowedconflicts']

            self.add_property(prop_obj)

            for property in extra_properties:
                self.add_property(property)

    def get_property(self, name):
        """
        Look up property on this module
        :param name: Regular expression needing to match the name of the property
        :return: Property if found, None elsewhere
        """
        return next((x for x in self.properties if name == x.name), None)

    def get_properties(self):
        '''
        :return: Collection of properties in this module
        '''
        return self.properties

    def activate_runtime(self):
        # Install default hooks
        for prop in self.properties:
            if isinstance(prop, ModuleProperty):
                if prop.inherit_options:
                    RuntimeModel.set_change_handler(prop, RuntimeModel.owning_module_property_callback, on_enable=True)
            if isinstance(prop, ModeProperty):
                RuntimeModel.set_change_handler(prop, RuntimeModel.module_mode_callback)
            if isinstance(prop, PinProperty):
                if prop.referenced_route is None:
                    RuntimeModel.set_change_handler(prop, RuntimeModel.pin_selection_callback)
                else:
                    RuntimeModel.configure_route_handler(prop)
            if prop.name == "owner":
                RuntimeModel.set_change_handler(prop, RuntimeModel.owner_changed_callback)
            if prop.name == "usedbysdk":
                RuntimeModel.set_change_handler(prop, RuntimeModel.module_usedbysdk_callback)
            if prop.name == "hiddenbysdk":
                RuntimeModel.set_change_handler(prop, RuntimeModel.module_hiddenbysdk_callback)
            if prop.name == "showadvanced":
                RuntimeModel.set_change_handler(prop, RuntimeModel.module_showadvanced_callback)
            if isinstance(prop, PRSChannelProperty):
                RuntimeModel.set_change_handler(prop, RuntimeModel.prs_channel_changed_callback, on_enable=True)

        # Install user hooks
        self.set_runtime_hooks()

    def set_runtime_hooks(self):
        """
        To be overridden by the implementing HAL Config module
        :return: None
        """
        pass

    def post_load(self):
        """
        To be overridden by the implementing HAL Config module
        :return: None
        """
        pass


def get_property(module_name, property_name):
    """
    Get a property model object by searching for owner and property name
    :param module_name: name string of module on which to look for the property
    :param property_name: name of the property
    :return: ExporterModel.Property (or superclass) if found, None else.
    """
    mod = next((m for m in module_list if m.name == module_name), None)
    if mod is None:
        return None
    prop = mod.get_property(property_name)
    return prop


def override_module(old, new):
    """
    Override a module in the global module_list with another instance
    :param old:
    :param new:
    :return:
    """
    global module_list
    if old.name != new.name:
        print("ERROR: Not replacing module with same module")
        return
    for k,v in enumerate(module_list):
        if v == old:
            module_list[k] = new


def create_global_available_modules_list(available_mod_list):
    for mod in available_mod_list:
        available_module_names_list.append(mod)


def mask_peripheral_selectors_with_module_list(module_names):
    global module_list
    for module in module_list:
        for property in module.properties:
            if isinstance(property, ModuleProperty):
                property.mask_with_module_list(list(module_names))
