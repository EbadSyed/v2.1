import os
import glob
import time
import Studio.halConfig as hal
import ezr32wg.PythonSnippet.RuntimeModel as RuntimeModel
import ezr32wg.PythonSnippet.ExporterModel as ExporterModel
import ezr32wg.PythonSnippet.Metadata as Metadata
import ezr32wg.modules.PIN.PIN_Snippets as PIN_Snippets
import ezr32wg.modules.PORTIO.PORTIO_Snippets as PORTIO_Snippets
import ezr32wg.halconfig.halconfig_dependency as dep

import ezr32wg.modules.ANTDIV.ANTDIV_behavior as ANTDIV_behavior
import ezr32wg.modules.BTL_BUTTON.BTL_BUTTON_behavior as BTL_BUTTON_behavior
import ezr32wg.modules.BUTTON.BUTTON_behavior as BUTTON_behavior
import ezr32wg.modules.CMU.CMU_behavior as CMU_behavior
import ezr32wg.modules.COEX.COEX_behavior as COEX_behavior
import ezr32wg.modules.EMU.EMU_behavior as EMU_behavior
import ezr32wg.modules.EXTFLASH.EXTFLASH_behavior as EXTFLASH_behavior
import ezr32wg.modules.EZRADIOPRO.EZRADIOPRO_behavior as EZRADIOPRO_behavior
import ezr32wg.modules.GPIO.GPIO_behavior as GPIO_behavior
import ezr32wg.modules.I2C0.I2C_behavior as I2C_behavior
import ezr32wg.modules.I2C1.I2C_behavior as I2C_behavior
import ezr32wg.modules.I2CSENSOR.I2CSENSOR_behavior as I2CSENSOR_behavior
import ezr32wg.modules.IOEXP.IOEXP_behavior as IOEXP_behavior
import ezr32wg.modules.LED.LED_behavior as LED_behavior
import ezr32wg.modules.LEUART0.LEUART_behavior as LEUART_behavior
import ezr32wg.modules.LEUART1.LEUART_behavior as LEUART_behavior
import ezr32wg.modules.PRS.PRS_behavior as PRS_behavior
import ezr32wg.modules.SERIAL.SERIAL_behavior as SERIAL_behavior
import ezr32wg.modules.SPIDISPLAY.SPIDISPLAY_behavior as SPIDISPLAY_behavior
import ezr32wg.modules.SPINCP.SPINCP_behavior as SPINCP_behavior
import ezr32wg.modules.TIMER0.TIMER_behavior as TIMER_behavior
import ezr32wg.modules.TIMER1.TIMER_behavior as TIMER_behavior
import ezr32wg.modules.TIMER2.TIMER_behavior as TIMER_behavior
import ezr32wg.modules.TIMER3.TIMER_behavior as TIMER_behavior
import ezr32wg.modules.UART0.UART_behavior as UART_behavior
import ezr32wg.modules.UART1.UART_behavior as UART_behavior
import ezr32wg.modules.UARTNCP.UARTNCP_behavior as UARTNCP_behavior
import ezr32wg.modules.USART1.USART_behavior as USART_behavior
import ezr32wg.modules.USART2.USART_behavior as USART_behavior
import ezr32wg.modules.VCOM.VCOM_behavior as VCOM_behavior
import ezr32wg.modules.VUART.VUART_behavior as VUART_behavior
import ezr32wg.modules.WDOG.WDOG_behavior as WDOG_behavior
PROFILE = True

@RuntimeModel.bind_document_load
def onLoad(state):
    # Prevent changed properties from enabling parent peripheral
    try:
        hal.registerDeviceOverride(hal.OVERRIDE_PERIPHERAL_AUTO_ENABLE, True)
    except:
        # Fall back to misspelled version of the function argument
        try:
            hal.registerDeviceOverride(hal.OVERRIDE_PERIPHRAL_AUTO_ENABLE, True)
        except:
            pass

    ExporterModel.create_global_available_modules_list(Metadata.available_modules_for_family)

    if PROFILE:
        start = time.time()

    familyobj = dep.Family(state.device.name)


    modInst = ANTDIV_behavior.ANTDIV('ANTDIV')
    modInst.load_halconfig_model(familyobj)

    modInst = BTL_BUTTON_behavior.BTL_BUTTON('BTL_BUTTON')
    modInst.load_halconfig_model(familyobj)

    modInst = BUTTON_behavior.BUTTON('BUTTON')
    modInst.load_halconfig_model(familyobj)

    modInst = CMU_behavior.CMU('CMU')
    modInst.load_halconfig_model(familyobj)

    modInst = COEX_behavior.COEX('COEX')
    modInst.load_halconfig_model(familyobj)

    modInst = EMU_behavior.EMU('EMU')
    modInst.load_halconfig_model(familyobj)

    modInst = EXTFLASH_behavior.EXTFLASH('EXTFLASH')
    modInst.load_halconfig_model(familyobj)

    modInst = EZRADIOPRO_behavior.EZRADIOPRO('EZRADIOPRO')
    modInst.load_halconfig_model(familyobj)

    modInst = GPIO_behavior.GPIO('GPIO')
    modInst.load_halconfig_model(familyobj)

    modInst = I2C_behavior.I2C('I2C0')
    modInst.load_halconfig_model(familyobj)

    modInst = I2C_behavior.I2C('I2C1')
    modInst.load_halconfig_model(familyobj)

    modInst = I2CSENSOR_behavior.I2CSENSOR('I2CSENSOR')
    modInst.load_halconfig_model(familyobj)

    modInst = IOEXP_behavior.IOEXP('IOEXP')
    modInst.load_halconfig_model(familyobj)

    modInst = LED_behavior.LED('LED')
    modInst.load_halconfig_model(familyobj)

    modInst = LEUART_behavior.LEUART('LEUART0')
    modInst.load_halconfig_model(familyobj)

    modInst = LEUART_behavior.LEUART('LEUART1')
    modInst.load_halconfig_model(familyobj)

    modInst = PRS_behavior.PRS('PRS')
    modInst.load_halconfig_model(familyobj)

    modInst = SERIAL_behavior.SERIAL('SERIAL')
    modInst.load_halconfig_model(familyobj)

    modInst = SPIDISPLAY_behavior.SPIDISPLAY('SPIDISPLAY')
    modInst.load_halconfig_model(familyobj)

    modInst = SPINCP_behavior.SPINCP('SPINCP')
    modInst.load_halconfig_model(familyobj)

    modInst = TIMER_behavior.TIMER('TIMER0')
    modInst.load_halconfig_model(familyobj)

    modInst = TIMER_behavior.TIMER('TIMER1')
    modInst.load_halconfig_model(familyobj)

    modInst = TIMER_behavior.TIMER('TIMER2')
    modInst.load_halconfig_model(familyobj)

    modInst = TIMER_behavior.TIMER('TIMER3')
    modInst.load_halconfig_model(familyobj)

    modInst = UART_behavior.UART('UART0')
    modInst.load_halconfig_model(familyobj)

    modInst = UART_behavior.UART('UART1')
    modInst.load_halconfig_model(familyobj)

    modInst = UARTNCP_behavior.UARTNCP('UARTNCP')
    modInst.load_halconfig_model(familyobj)

    modInst = USART_behavior.USART('USART1')
    modInst.load_halconfig_model(familyobj)

    modInst = USART_behavior.USART('USART2')
    modInst.load_halconfig_model(familyobj)

    modInst = VCOM_behavior.VCOM('VCOM')
    modInst.load_halconfig_model(familyobj)

    modInst = VUART_behavior.VUART('VUART')
    modInst.load_halconfig_model(familyobj)

    modInst = WDOG_behavior.WDOG('WDOG')
    modInst.load_halconfig_model(familyobj)


    if PROFILE:
        stop = time.time()
        print("  construction of all modules completed in {:.3f} ms".format((stop - start) * 1000))
        start = time.time()

    # Do the hook installing after all modules have initialized
    PIN_Snippets.activate_runtime()
    PORTIO_Snippets.activate_runtime()

    for modInst in ExporterModel.module_list:
        modInst.activate_runtime()

    PORTIO_Snippets.onLoad(state)

    if PROFILE:
        stop = time.time()
        print("  activate_runtime() for all modules completed in {:.3f} ms".format((stop - start) * 1000))
