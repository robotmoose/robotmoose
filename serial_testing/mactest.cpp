kern_return_t find_modems(io_iterator_t* matchingServices)
{
	CFMutableDictionaryRef classesToMatch=IOServiceMatching(kIOSerialBSDServiceValue);

	if(classesToMatch==NULL)
		printf("IOServiceMatching returned a NULL dictionary.\n");
	else
		CFDictionarySetValue(classesToMatch,CFSTR(kIOSerialBSDTypeKey),CFSTR(kIOSerialBSDModemType));

	kern_return_t kernResult=IOServiceGetMatchingServices(kIOMasterPortDefault,classesToMatch,matchingServices);

	if(KERN_SUCCESS!=kernResult)
	{
		printf("IOServiceGetMatchingServices returned %d\n",kernResult);
		return kernResult;
	}
}