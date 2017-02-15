	SECTION	entry:CODE:ROOT(4)
	EXTERN	module_entry
	EXTERN	module_exit
	DATA
	DCD		module_entry
	DCD		module_exit
	DCD		MODULE_SIZE
	DCD		MODULE_NAME
	END
