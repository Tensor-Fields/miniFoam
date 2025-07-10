// Create time
Time runTime
(
    "controlDict",
	   args.rootPath(),
	   args.caseName(),
	   "system",
	   "constant",
	   !args.optionFound("noFunctionObjects")
);
