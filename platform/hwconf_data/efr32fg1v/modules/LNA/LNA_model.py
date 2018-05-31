from . import halconfig_types as types
from . import halconfig_dependency as dep

name = "LNA"
displayname = "External LNA"
compatibility = dep.Dependency(platform=dep.Platform.SERIES1, mcu_type=dep.McuType.RADIO)  # EFR32
category = " Radio"
studio_module = {
    "basename" : "SDK.HAL.LNA",
    "modules" : [types.StudioFrameworkModule("BASE", [types.Framework.ZNET, types.Framework.THREAD])],
    }
enable = {
    "define": "HAL_LNA_ENABLE",
    "description": "Enable LNA",
}
options = {
    "BSP_LNA_TXRX": {
        "type": types.PRSChannelLocation("BSP_LNA_TXRX", custom_name="LNA_TXRX"),
        "description": "External LNA TX/RX PRS channel",
        "longdescription": "PRS channel for TX/RX LNA control",
    },
    "BSP_LNA_SLEEP": {
        "type": types.PRSChannelLocation("BSP_LNA_SLEEP", custom_name="LNA_SLEEP"),
        "description": "External LNA sleep PRS channel",
        "longdescription": "PRS channel for external LNA sleep control",
    },
}