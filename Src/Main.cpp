import ArxConverter;


Int32 main(Int32 argc, CString argv[])
{
	try
	{
		ArxConverter converter{ argc, argv };

		converter.start();
	}
	catch (...)
	{
		return 1;
	}

	return 0;
}