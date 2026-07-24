set(MCU_VARIANT mspm0c511x)
set(MCU_SPECIFIC mspm0c5116)
set(JLINK_DEVICE ${MCU_VARIANT})

function(update_board TARGET)
  target_compile_definitions(${TARGET} PUBLIC
    __MSPM0C511x__
    )
endfunction()
