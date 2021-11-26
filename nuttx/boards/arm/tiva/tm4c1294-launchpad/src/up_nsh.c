int nsh_archinitialize(void)
{
#ifdef CONFIG_ARCH_LEDS
up_leds();
#endif
return 1;
}