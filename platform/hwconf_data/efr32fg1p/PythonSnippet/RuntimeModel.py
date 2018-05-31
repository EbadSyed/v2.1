"""
Collection of functions to exercise the model at Studio runtime
"""

import Studio
import ExporterModel
import Metadata

class State(object):
    def __init__(self, device=None, module=None, mode=None, args=None):
        self.changes = []
        self.device = device
        self.studio_module = module
        self.mode = mode
        self.args = args


def bind_document_load(func):
    '''
    Binds the decorated function to run whenever the document is loaded
    :param func: Python function func(device) to run on document load
    '''
    def realCall(event, device):
        state = State(device=device)
        func(state)
        return state.changes
    ##END: realCall()

    oFilter = Studio.halConfig.newObjectNameEventFilter('.*')
    Studio.halConfig.addEventHandler(Studio.halConfig.DOCUMENT_LOAD, oFilter, realCall)

    return func


def set_change_handler(property, func, on_enable=False, args=None):
    """
    Sets up a change handler on the given property
    :param property: ExporterModel Property on which the callback will get registered
    :param func: Python function of the form func(studio_module, exportmodel_prop)
    :param on_enable: If true, register callback on enable checkbox too
    :return: property
    """

    def realCall(event, prop):
        state = State(module=prop.object, mode=prop.object.getMode(), args=args)
        # Execute bound function
        if on_enable:
            is_enable = True if prop.propertyId == 'ABPeripheral.included' else False
            func(state.studio_module, property, state, is_enable)
        else:
            func(state.studio_module, property, state)
        # Return possibly changed properties to Studio for execution
        return state.changes

    prop = property.id
    if on_enable:
        prop = "({}|ABPeripheral.included)".format(prop)

    eFilter = Studio.halConfig.newObjectPropertyEventFilter(property.parent.name, prop)
    Studio.halConfig.addEventHandler(Studio.halConfig.PROPERTY_CHANGE, eFilter, realCall)
    return property


def set_change_handler_regex(module_re, property_re, func, args = None):
    """
    Sets up a change handler on the given modules' property/ies
    :param module_re: Regular expression matching the module(s) on which to look for properties
    :type module_re: str
    :param property_re: Regular expression matching the property/ies
    :type property_re: str
    :param func: Python callback function of form func(studio_module, studio_prop, args)
    :param args: arguments to pass to callback
    :return: None
    """
    def realCall(event, prop):
        state = State(module=prop.object, mode=prop.object.getMode(), args=args)
        # Execute bound function
        func(state.studio_module, prop, state)
        # Return possibly changed properties to Studio for execution
        return state.changes

    eFilter = Studio.halConfig.newObjectPropertyEventFilter(module_re, property_re)
    Studio.halConfig.addEventHandler(Studio.halConfig.PROPERTY_CHANGE, eFilter, realCall)


def get_module_checked(module, mode):
    if isinstance(module, str):
        module = get_studio_module_by_name(module, mode)

    return module.getProperty("ABPeripheral.included")


def set_module_checked(module, checked, state, readonly=False):
    if isinstance(module, str):
        module = get_studio_module_by_name(module, state.mode)

    property = module.getPropertyReference("ABPeripheral.included")
    state.changes.append(Studio.halConfig.newPropertySetting(property, checked))
    state.changes.append(Studio.halConfig.newIsReadOnlySetting(property, readonly))


def set_module_visible(module, state, visible):
    if isinstance(module, str):
        module = get_studio_module_by_name(module, state.mode)

    property = module.getPropertyReference("ABPeripheral.included")
    state.changes.append(Studio.halConfig.newIsHiddenSetting(property, not visible))


def get_property_reference(property, module):
    """
    Tries to get a Studio property reference from a 'property' object through type-checking
    :param property: property-like object
    :param module: Studio module object
    :return: Studio property ID or None
    """
    if isinstance(property, ExporterModel.Property):
        return module.ref(property.id)
    elif isinstance(property, str):
        if '.' in property:
            return module.ref(property)
        else:
            property = ExporterModel.get_property(module.name, property)
            if property is not None:
                return module.ref(property.id)
            else:
                return None
    else:
        return property


def get_property_model(property, module):
    """
    Tries to get a property model object from a 'property' object
    :param property: property-like object
    :return: Model property object or None
    """
    if isinstance(property, ExporterModel.Property):
        return property
    elif isinstance(property, str):
        if '.' in property:
            return ExporterModel.get_property(module.name, property.split('.')[1])
        else:
            return ExporterModel.get_property(module.name, property)
    else:
        # Studio properties get tricky
        return ExporterModel.get_property(module.name, property.id.split('.')[1])


def get_property_value(property, module):
    """
    Get the runtime setting of a property
    :param module: Studio Module object.
    :param property: ExporterModel Property, Studio property or property string
    :return: String value of the property
    """
    if module is None:
        return None

    prop = get_property_reference(property, module)
    if prop is not None:
        return str(prop.get())
    else:
        return None


def get_property_define(property, module):
    """
    Get the runtime #define setting of a property
    :param module: Studio Module object.
    :param property: ExporterModel Property, Studio property or property string
    :return: String value of the value property
    """
    if module is None:
        return None

    prop = get_property_reference(property, module)
    if prop is not None:
        return str(prop.get())

    return ""


def get_property_values(property, module):
    """
    Get the possible values for a property
    :param module: Studio Module object.
    :param property: ExporterModel Property, Studio property or property string
    :return: List of possible values. Empty list if not an enum-based property.
    """
    if module is None:
        return []

    propref = get_property_reference(property, module)

    if propref.getEnumValues():
        return [str(p) for p in propref.getEnumValues()]
    else:
        return []


def set_property_value(property, value, state, module=None):
    """
    Set the runtime setting of a property
    :param property: ExporterModel Property, Studio property or property string
    :param value: Value to set the property to
    :param state: Context variable
    :param module: Studio Module object. If not given, module is taken from state
    :return: None
    """
    if not module:
        module = state.studio_module
    propref = get_property_reference(property, module)

    state.changes.append(Studio.halConfig.newPropertySetting(propref, value))

    return None


def set_signal_value(signal, value, state):
    """
    Set the runtime value of a signal
    :param signal: Studio signal
    :param value: Value to set the signal to
    :param state: Context variable
    :return: None
    """
    state.changes.append(Studio.halConfig.newSignalSetting(signal, value))

    return None


def set_property_readonly(property, readonly, state, module=None):
    """
    Set a property to read-only (greyed out)
    :param property: ExporterModel Property, Studio property or property string
    :param readonly: True if property needs to be read-only
    :param state: Context variable
    :param module: Studio Module object. If not given, module is taken from state
    :return: None
    """
    if not module:
        module = state.studio_module
    propref = get_property_reference(property, module)

    state.changes.append(Studio.halConfig.newIsReadOnlySetting(propref, readonly))

    return None


def get_property_readonly(property, module):
    """
    Get the read-only setting of a property
    :param property: ExporterModel Property, Studio property or property string
    :param module: Studio Module object.
    :return: True if property is set to read-only
    """
    #TODO: implement this function?
    pass


def set_property_hidden(property, hidden, state, module=None):
    """
    Set a property hidden (invisible)
    :param property: ExporterModel Property, Studio property or property string
    :param readonly: True if property needs to be read-only
    :param state: Context variable
    :param module: Studio Module object. If not given, module is taken from state
    :return: None
    """
    if not module:
        module = state.studio_module
    propref = get_property_reference(property, module)

    state.changes.append(Studio.halConfig.newIsHiddenSetting(propref, hidden))

    return None


def get_property_hidden(property, module):
    """
    Get the hidden setting of a property
    :param property: ExporterModel Property, Studio property or property string
    :param module: Studio Module object.
    :return: True if property is set to hidden
    """
    #TODO: implement this function?
    pass


def set_module_ownership(owner_name, owned_name, possible_owned_list, state, greyout=False):
    """
    Tries to set the owner property on a peripheral to the owner module
    :param owner_name: Name of the owner module
    :param owned_name: Name of the requested owned module
    :param possible_owned_list: List over all modules the owner module might have had ownership of
    :param state: Context variable
    :param greyout: Disable owned module's properties from user changes
    :return: True if owner_name is the only owner, False if the requested module is also owned by another module
    """
    # Changed selection to no longer own module
    for owned_mod in [m for m in possible_owned_list if m not in ["None", owned_name]]:
        prev_owner = get_property_value("owner", module=get_studio_module_by_name(owned_mod, state.mode))
        if prev_owner and ', ' in prev_owner:
            owner_list = prev_owner.split(", ")
            if owner_name in owner_list:
                    owner_list.remove(owner_name)
                    new_owner = ", ".join(owner_list)
                    set_property_value("owner", new_owner, state=state, module=get_studio_module_by_name(owned_mod, state.mode))
                    set_module_checked(owned_mod, get_module_checked(owned_mod, state.mode), state, readonly=True)
        elif prev_owner and prev_owner == owner_name:
            set_property_value("owner", "", state=state, module=get_studio_module_by_name(owned_mod, state.mode))
            set_module_checked(owned_mod, get_module_checked(owned_mod, state.mode), state, readonly=False)
        if greyout:
            # TODO: return module properties back to changeable state
                pass

    if owned_name == 'None':
        return True
    else:
        # try to own requested module
        prev_owner = get_property_value("owner", module=get_studio_module_by_name(owned_name, state.mode))
        if not prev_owner:
            # Nobody owns the module; take it
            set_property_value("owner", owner_name, state=state, module=get_studio_module_by_name(owned_name, state.mode))
            set_module_checked(owned_name, True, state, readonly=True)
            # TODO: set enable checkbox to read-only and possibly properties too
            return True
        elif owner_name in prev_owner:
            # We already own the module
            return True
        else:
            # Somebody else owns the module - setting double ownership
            set_property_value("owner", "{0}, {1}".format(prev_owner, owner_name), state=state, module=get_studio_module_by_name(owned_name, state.mode))
            return False


def get_studio_module_by_name(module_name, mode):
    for p in mode.getPeripherals():
        if p.name == module_name:
            return p
    return None


def get_studio_pin_modules(mode):
    pin_list = []
    for port in mode.portIO.getPortBanks():
        for pin in port.getPins():
            pin_list.append(pin)

    return pin_list


def owning_module_property_callback(studio_module, model_property, state, is_enable):
    owner_name = model_property.parent.name + model_property.define_name_postfix
    if get_module_checked(studio_module, studio_module.getMode()):
        new_owned_name = get_property_value(model_property, module=studio_module)
        print("Module prop on {} changed to {}".format(owner_name, new_owned_name))
        if set_module_ownership(owner_name, new_owned_name, get_property_values(model_property, module=studio_module), state):
            # Studio_module is only owner, so apply owner-decided mode for owned module
            if model_property.owned_mode is not None and new_owned_name != "None":
                print("Changing mode on {} to {}".format(new_owned_name, model_property.owned_mode))
                prop_id = get_property_id_from_label("mode", new_owned_name, studio_module.getMode())
                if prop_id:
                    prop_name = prop_id.split('.')[1]
                    set_property_value(str(prop_name),
                                       model_property.owned_mode,
                                       state=state,
                                       module=get_studio_module_by_name(new_owned_name, state.mode))
        else:
            # Successfully added multiple owners. Mode is not changed for owned module
            pass

    else:
        set_module_ownership(owner_name, 'None',
                             get_property_values(model_property, module=studio_module),
                             state)


def module_mode_callback(studio_module, model_property, state):
    owner_module_name = model_property.parent.name
    new_mode_name = get_property_define(model_property, module=studio_module)

    show_advanced = get_property_value(model_property.parent.get_property('showadvanced'), studio_module) == '1'

    print("Mode on {} changed to {} (advanced = {})".format(owner_module_name, new_mode_name, show_advanced))

    for modprop in model_property.parent.get_properties():
        if not show_advanced and modprop.is_advanced:
            # Do nothing to advanced properties if not in advanced mode
            continue
        if modprop.mode is not None:
            # Setting visibility for relevant properties
            if isinstance(modprop.mode, list):
                if new_mode_name in modprop.mode:
                    set_property_hidden(modprop, False, state=state)
                else:
                    set_property_hidden(modprop, True, state=state)
            else:
                if modprop.mode == new_mode_name:
                    set_property_hidden(modprop, False, state=state)
                else:
                    set_property_hidden(modprop, True, state=state)


def pin_selection_callback(studio_module, model_property, state):
    pin_instance = get_property_value(model_property, module=studio_module)
    # remove the pin name on other GPIO
    for instance in get_studio_pin_modules(studio_module.getMode()):
        old_val = get_property_value('ports.settings.halpinname', module=instance)
        if instance.name == pin_instance:
            if old_val:
                set_property_value('ports.settings.halpinname', "{},{}".format(old_val,model_property.name), state=state, module=instance)
            else:
                set_property_value('ports.settings.halpinname', model_property.name, state=state, module=instance)
        elif model_property.name in old_val:
            if ',' in old_val:
                vals = old_val.split(',')
                vals.remove(model_property.name)
                set_property_value('ports.settings.halpinname', ','.join(vals), state=state, module=instance)
            else:
                set_property_value('ports.settings.halpinname', '', state=state, module=instance)

def route_selection_callback(studio_module, pin_property, state):
    """
    Callback when a PinProperty changed in some module, requiring PORTIO to be updated
    :param studio_module:  The peripheral module that contains the changed PinProperty
    :param pin_property: The changed PinProperty (ExporterModel object)
    :return:
    """
    mod = pin_property.referenced_module
    route_name = pin_property.referenced_route
    selected_pin = get_property_value(pin_property, module=studio_module)

    portio = studio_module.mode.portIO

    # Get the selectors for the given peripheral
    selectors = filter(lambda x: str(x.module).split('::')[1] == mod, portio.selectors)
    route = None

    for selector in selectors:
        # Search all selectors to find the one containing our route, and select the route
        route = filter(lambda x: str(x.name) == route_name, selector.routes)
        if route:
            # Selector contains route matching desired signal
            enable = route[0].enablePropertyReference
            if selected_pin != "Disabled":
                # Setting pin to something not-Disabled means we need to update the location on the route
                if selector.locationPropertyReference:
                    try:
                        # Selector has multiple locations for each route -- find the location number matching the
                        # pin name for our selected route
                        location = filter(lambda x: str(x.pin).split('::')[1] == selected_pin, route[0].locations)[0]
                        # Set location on referenced location property
                        set_property_value(selector.locationPropertyReference, str(location.locationNumber), module=portio, state=state)
                    except Exception as e:
                        print("WARNING :: Selected a pin with no valid location for route {}::{}".format(e.message, mod, route_name))
                        # Switch Enable property of route to Disabled
                        set_property_value(enable, "Disabled", module=portio, state=state)
                        break
                else:
                    # Selector has only a single location per route -- do nothing
                    pass
                # Switch Enable property of route to Enabled
                set_property_value(enable, "Enabled", module=portio, state=state)
            else:
                # Switch Enable property of route to Disabled
                set_property_value(enable, "Disabled", module=portio, state=state)
            break

    if not route:
        print("ERROR: Route {} does not exist in module {}".format(mod, route_name))


def module_usedbysdk_callback(studio_module, model_property, state):
    used = get_property_value(model_property, module=studio_module)
    set_module_checked(studio_module, used == '1', state, readonly=(used == '1'))

    for property_id in studio_module.getPropertyIds():
        prop_ref = studio_module.ref(property_id)
        set_property_readonly(prop_ref, True, state=state)


def module_hiddenbysdk_callback(studio_module, model_property, state):
    hidden = get_property_value(model_property, module=studio_module)
    # TODO: Actually hide modules when this thing starts working
    # set_module_visible(studio_module, studio_module.getMode(), not (hidden == 'True'))
    set_module_checked(studio_module, hidden != '1', state, readonly=True)

    for property_id in studio_module.getPropertyIds():
        prop_ref = studio_module.ref(property_id)
        set_property_hidden(prop_ref, True, state=state)


def module_showadvanced_callback(studio_module, model_property, state):
    show_advanced = get_property_value(model_property, module=studio_module) == '1'

    for property in model_property.parent.get_properties():
        if property.is_advanced and show_advanced:
            set_property_hidden(property, False, state)


def route_selection_portio_callback(portio, portio_property, state):
    """
    Callback when a PORTIO changed on a module that has a PinProperty shadowing PORTIO
    :param portio:           Reference to the PORTIO studio module
    :param portio_property:  The changed PORTIO property object
    :param args:             Arbitrary arguments -- used to give us the PinProperty that needs updating
    :return:
    """
    pin_property = state.args['model']

    mod = pin_property.referenced_module
    signal = pin_property.referenced_route

    if get_property_value(state.args['enable'], module=portio) == "Enabled":
        # PORTIO property is enabled -- need to find the new pin name from the new location set in PORTIO
        # Get the selectors for the given peripheral

        selectors = filter(lambda x: str(x.module).split('::')[1] == mod, portio.selectors)
        location = None

        for selector in selectors:
            # Search all selectors to find the one containing our route, and select the route
            route = filter(lambda x: str(x.name) == signal, selector.routes)
            if route:
                # Selector contains route matching desired signal
                # Get the selected location from the route
                if selector.locationPropertyReference:
                    # Selector has multiple locations for each route -- find the location object matching the
                    # location number for our selected route
                    try:
                        location = filter(lambda x: str(x.locationNumber) == selector.locationPropertyReference.get(), route[0].locations)[0]
                    except:
                        # No pin on this location
                        print("WARNING: {} has no pin on location {}".format(mod, signal))
                        set_property_value(pin_property,
                                           "Disabled",
                                           module=get_studio_module_by_name(pin_property.parent.name, state.mode),
                                           state=state)
                        return
                else:
                    # Selector has only a single location per route -- choose the first (and only one)
                    location = route[0].locations[0]
                break
        # Get the pin name from the location object
        if location:
            pin = str(location.pin).split('::')[-1]
            set_property_value(pin_property,
                               pin,
                               module=get_studio_module_by_name(pin_property.parent.name, state.mode),
                               state=state)
    else:
        # PORTIO property was changed to disabled; disable PinProperty too
        set_property_value(pin_property,
                           "Disabled",
                           module=get_studio_module_by_name(pin_property.parent.name, state.mode),
                           state=state)


def configure_route_handler(model_property):
    # If the property changes on the Peripheral Properties page, update port IO
    set_change_handler(model_property, route_selection_callback)

    mod = model_property.referenced_module
    route = model_property.referenced_route

    # If port IO changes, update the property
    location = "portio.{0}.location..+".format(mod.lower(), route.lower())
    enable = "portio.{0}.enable.{1}".format(mod.lower(), route.lower())
    set_change_handler_regex("PORTIO", location, route_selection_portio_callback, {"model": model_property, "enable": enable})
    set_change_handler_regex("PORTIO", enable, route_selection_portio_callback, {"model": model_property, "enable": enable})


def new_problem_group(prop, studio_module, state, src=""):
    """ Creates a problem group. Old problems in the group will be erased when the problem group is called again """
    if prop is not None:
        identifier = "{}__{}__{}".format(prop.getObject().getName(), prop.getId().split("::")[1], src)
    else:
        identifier = "{}__{}".format(studio_module.getName(), src)
    state.changes.append(Studio.halConfig.newProblemGroup(studio_module.getMode(), identifier))


def new_error(prop, msg, state):
    """ Causes the passed error msg to be applied to the the passed property when realCall returns """
    state.changes.append(Studio.halConfig.newError(prop, msg))


def new_warning(prop, msg, state):
    """ Causes the passed warning msg to be applied to the the passed property when realCall returns """
    state.changes.append(Studio.halConfig.newWarning(prop, msg))


def new_info(prop, msg, state):
    """ Causes the passed problem msg to be applied to the the passed property when realCall returns """
    state.changes.append(Studio.halConfig.newProblem(prop, 1, msg))


def owner_changed_callback(studio_module, property, state):
    """ Generates warning for current module if there are multiple owners """
    owner = get_property_value(property, studio_module)
    prop_ref = get_property_reference(property, studio_module)
    new_problem_group(prop_ref, studio_module, state, src="owner_changed_callback")
    if ', ' in owner:
        msg = "{0} has multiple owners: {1}".format(studio_module.getName(), owner)
        new_info(prop_ref, msg, state)


def get_property_id_from_label(prop_label, studio_module_name, mode):
    """
    Searching through properties, returning the property ID with correct label, else None
    :param prop_label: string containing property label (i.e. the label in Studio XML files)
    :param studio_module_name: string representing a studio module
    :param mode: Mode to get Studio module from
    :return: Property ID, or None
    """
    studio_module = get_studio_module_by_name(studio_module_name, mode)
    for property_id in studio_module.getPropertyIds():
        prop_ref = studio_module.ref(property_id)
        if prop_ref.getLabel() == prop_label:
            return property_id
    return None


def prs_channel_changed_callback(studio_mod, channel_property, state, is_enable):
    """
    Callback for PRSChannelProperty class.
    Hides and unhides the the prs output pin selectors, based on the choice of channel.
    Sets relevant custom names in the PRS module
    """
    prs_channel_count = Metadata.get_prs_chan_with_gpio_count()
    selected_channel = get_property_value(channel_property, module=studio_mod)
    prs_studio_module = get_studio_module_by_name("PRS", studio_mod.getMode())
    subcategories = [sub_cat for sub_cat in prs_studio_module.getCategories()[0].getSubcategories()]

    # Removing old custom name
    custom_name = channel_property.custom_name if channel_property.custom_name else studio_mod.getName()
    for subcategory in subcategories:
        for property_id in subcategory.getPropertyIds():
            if str(subcategory.getLabel()) != "CH{} properties".format(selected_channel):
                # Remove custom name if the selection has changed
                prop_ref = prs_studio_module.ref(property_id)
                if "custom_name" in property_id and get_property_value(prop_ref, prs_studio_module) == custom_name:
                    set_property_value(prop_ref, "", module=prs_studio_module, state=state)
                    set_property_readonly(prop_ref, readonly=False, module=prs_studio_module, state=state)
            elif is_enable and not get_module_checked(module=studio_mod, mode=studio_mod.getMode()):
                # If callback is from disabling the module, always remove custom name from PRS module
                prop_ref = prs_studio_module.ref(property_id)
                if "custom_name" in property_id and get_property_value(prop_ref, prs_studio_module) == custom_name:
                    set_property_value(prop_ref, "", state, module=prs_studio_module)
                    set_property_readonly(prop_ref, readonly=False, state=state, module=prs_studio_module)

    # if module is enabled, set properties hidden/unhidden and custom names
    if get_module_checked(module=studio_mod, mode=studio_mod.getMode()):
        # Checking the selected value, to determine what to do
        if selected_channel == "Disabled":
            # Hiding all properties, except prs_disabled_chn_pin which should be displayed when selection is "Disabled"
            set_property_hidden("prs_disabled_chn_{}_pin".format(channel_property.custom_name if channel_property.custom_name else ""),
                                hidden=False,
                                state=state)
            for i in range(prs_channel_count):
                set_property_hidden(channel_property.name.replace("_CHANNEL", str(i)),
                                    hidden=True,
                                    state=state)
        else:
            # Set custom name in PRS module, or return channel property to Disabled if custom name already set
            for subcategory in subcategories:
                label = str(subcategory.getLabel())
                if label == "CH{} properties".format(selected_channel):
                    for property_id in subcategory.getPropertyIds():
                        if "custom_name" in property_id:
                            custom_name_prop_ref = prs_studio_module.ref(property_id)
                            if get_property_value(custom_name_prop_ref, prs_studio_module) != "" and \
                                    get_property_value(custom_name_prop_ref, prs_studio_module) != custom_name:
                                # If channel is already taken, show warning
                                new_warning(get_property_reference(channel_property, studio_mod),
                                            "PRS channel already reserved for {}".format(get_property_value(custom_name_prop_ref, prs_studio_module)),
                                            state)
                            else:
                                # Set custom name and make it readonly
                                set_property_value(custom_name_prop_ref,
                                                   custom_name,
                                                   module=prs_studio_module,
                                                   state=state)
                                set_property_readonly(custom_name_prop_ref,
                                                      readonly=True,
                                                      module=prs_studio_module,
                                                      state=state)

            # Hide and unhide the different properties depending on channel selection
            selected_channel = int(selected_channel.replace("CH", ""))
            for i in range(prs_channel_count):
                set_property_hidden("prs_disabled_chn_{}_pin".format(channel_property.custom_name if channel_property.custom_name else ""),
                                    hidden=True,
                                    state=state)
                hidden = False if i == selected_channel else True
                set_property_hidden(channel_property.name.replace("_CHANNEL", str(i)),
                                    hidden=hidden,
                                    state=state)
