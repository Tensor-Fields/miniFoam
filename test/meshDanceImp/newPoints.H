// Calc/Assume new mesh points (only internal points, not boundary points)
pointField avgPoints(const fvMesh& mesh, const Time& runTime)
{
	/*
    // Find max coordinates
    scalar xMax = -GREAT;
    scalar yMax = -GREAT;
    scalar zMax = -GREAT;

    scalar xMin = GREAT;
    scalar yMin = GREAT;
    scalar zMin = GREAT;

    forAll(mesh.points(), pointI)
    {
        xMax = max(mesh.points()[pointI][0], xMax);
        yMax = max(mesh.points()[pointI][1], yMax);
        zMax = max(mesh.points()[pointI][2], zMax);

        xMin = min(mesh.points()[pointI][0], xMin);
        yMin = min(mesh.points()[pointI][1], yMin);
        zMin = min(mesh.points()[pointI][2], zMin);
    }

    scalar scale(5e-3);
	*/

    // Avoid moving boundary points
	pointField mp(mesh.points());

	pointMesh pMesh(mesh);

	//int N = mesh.nPoints();

	pointVectorField allPoints
	(
		IOobject
		(
	        "points",
			runTime.timeName(),
		    mesh,
		    IOobject::NO_READ,
		    IOobject::NO_WRITE
		),
		pMesh,
        dimensionedVector("points", dimless, vector::zero)
	);
	
	autoPtr<pointVectorField::InternalField> points(new pointVectorField::InternalField(allPoints.internalField()));
	//(
	//	IOobject
	//	(
	//        "points",
	//		runTime.timeName(),
	//	    mesh,
	//	    IOobject::NO_READ,
	//	    IOobject::NO_WRITE
	//	),
	//	pMesh,
    //    dimensionedVector("points", dimless, vector::zero)
	//);
	//autoPtr<pointVectorField> points(new pointVectorField);

	forAll(allPoints.internalField(), pointI)
	{
	    points()[pointI] = mp[pointI];
	}

	//pointVectorField points(pointsF.internalField());

	const labelListList& pp = mesh.pointPoints();

    forAll(points(), pointI)
    {
		/*
		if (pointI > mesh.points().size())
		{
		    FatalErrorIn("point number") << exit(FatalError);
		}
		*/

		/*
        if 
    	(
             points[pointI][0] != xMax || points[pointI][0] != xMin
          || points[pointI][1] != yMax || points[pointI][1] != yMin
          || points[pointI][2] != zMax || points[pointI][2] != zMin
    	)
		*/

		const labelList&  connectedPoints = pp[pointI];

		int pc = 1;

		forAll(connectedPoints, pI)
		{
		    points()[pointI][0] += points()[connectedPoints[pI]][0];
		    points()[pointI][1] += points()[connectedPoints[pI]][1];
		    points()[pointI][2] += points()[connectedPoints[pI]][2];

			pc++;
		}

		points()[pointI][0] /= pc;
		points()[pointI][1] /= pc;
		points()[pointI][2] /= pc;
    }

	int ppp = 0;

	forAll(points(), pointI)
	{
	   mp[pointI] =  points()[pointI];
	   ppp++;
	}

	/*
	for(int pointI = ppp; pointI<N; ++pointI)
	{
	   mp[pointI] = allPoints[pointI];
	   ppp++;
	}
	*/

	return mp;
}

pointField newPoints(const fvMesh& mesh)
{
    // Find max coordinates
    scalar xMax = -GREAT;
    scalar yMax = -GREAT;
    scalar zMax = -GREAT;

    scalar xMin = GREAT;
    scalar yMin = GREAT;
    scalar zMin = GREAT;

    forAll(mesh.points(), pointI)
    {
        xMax = max(mesh.points()[pointI][0], xMax);
        yMax = max(mesh.points()[pointI][1], yMax);
        zMax = max(mesh.points()[pointI][2], zMax);

        xMin = min(mesh.points()[pointI][0], xMin);
        yMin = min(mesh.points()[pointI][1], yMin);
        zMin = min(mesh.points()[pointI][2], zMin);
    }

    scalar scale(5e-3);

    tmp<pointField> pfPtr(new pointField(mesh.points()));

    forAll(pfPtr(), pointI)
    {
    	// Avoid moving boundary points
		if (pointI > mesh.points().size())
		{
		    FatalErrorIn("point number") << exit(FatalError);
		}

        if 
    	(
             pfPtr()[pointI][0] != xMax || pfPtr()[pointI][0] != xMin
          || pfPtr()[pointI][1] != yMax || pfPtr()[pointI][1] != yMin
          || pfPtr()[pointI][2] != zMax || pfPtr()[pointI][2] != zMin
    	)
        pfPtr()[pointI][0] = mesh.points()[pointI][0] * (1 - (xMax - mesh.points()[pointI][0])/(VSMALL + xMax) * scale);
        pfPtr()[pointI][1] = mesh.points()[pointI][1] * (1 - (yMax - mesh.points()[pointI][1])/(VSMALL + yMax) * scale);
        pfPtr()[pointI][2] = mesh.points()[pointI][2] * (1 - (zMax - mesh.points()[pointI][2])/(VSMALL + zMax) * scale);
    }

	return pfPtr();
}

/// Another point motion engine
//#include "readNewPoints.H"
pointField deltaPoints(const fvMesh& startMesh, const fvMesh& targetMesh)
{
    /// Calculate displacement increment
    scalar nTimeSteps = 
      (
          startMesh.time().endTime().value() 
        - startMesh.time().startTime().value()
      ) / startMesh.time().deltaT().value();

    return (targetMesh.points() - startMesh.points()) / nTimeSteps;
}

/// Time-step adaptive point increment, returns newPoints, not point increments
const pointField incrementPoints
(
    const fvMesh& startMesh, 
    const vectorField overallMotion
)
{
    /// Calculate displacement increment
    bool debug = false;

    if (debug)
    {
        Info<< "startPoints"
            << nl << startMesh.points()
            << endl;
            
        Info<< "deltaT"
            << nl << startMesh.time().deltaT().value() 
            << endl;

        Info<< "endTime:"
            << nl << startMesh.time().endTime().value() 
            << endl;

        Info<< "starTime:"
            << nl << startMesh.time().startTime().value()
            << endl;

        Info<< "overall motion:"
            << nl << overallMotion
            << endl;
    }

    return (
               startMesh.points() + startMesh.time().deltaT().value() /
                   (
                       startMesh.time().endTime().value() 
                     - startMesh.time().startTime().value()
                   ) * overallMotion
           );       
}

/// Calculate the vectorField that moves startMesh to endMesh
vectorField overallMotion(const fvMesh& startMesh, const fvMesh& targetMesh)
{
    /// Calculate overall displacement increment
    return (targetMesh.points() - startMesh.points());
}
