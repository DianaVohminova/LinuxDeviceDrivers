savedcmd_span_driver.mod := printf '%s\n'   span_driver.o | awk '!x[$$0]++ { print("./"$$0) }' > span_driver.mod
