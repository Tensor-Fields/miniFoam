template<class Type>
void ReadFields
(
    const IOobjectList& objects
)
{
    word fieldClassName
	(
	    GeometricField<Type, fvPatchField, volMesh>::typeName
	);

	IOobjectList fields = objects.lookupClass(fieldClassName);

	for
	(
	    IOobjectList::iterator fieldIter = fields.begin();
		fieldIter !=fields.end();
		++fieldIter
	)
	{
	    IOobject fieldIOobject
		(
		    fieldIter()->name(),
			mesh.time().timeName(),
			mesh,
			IOobject::MUST_READ
		);

		if (fieldIOobject.headerOk())
		{
		    Info<< "    Reading "
			    << fieldIter()->name()
				<< endl;

			GeometricField<Type, fvPatchField, volMesh> 
			fieldIter()->name()
			(
			    *fieldIter(),
				mesh
			);
		}
	}
}
//#include "IOobjectList.H"
//
//IOobjectList
//CreateIOobjects
//(
//    const Time& runTime, // To read the startTimeDir from
//    const fvMesh& mesh
//)
//{
//    const fileNameList& fieldNames = readDir(runTime.timeName());
//
//    IOobjectList objects(fieldNames.size());
//
//    forAll(fieldNames, fieldNameI)
//    {
//        word fieldName = fieldNames[fieldNameI].name();
//
//        //Info << "field found: " << fieldName << endl;
//
//        objects[fieldNameI] 
//        = 
//        IOobject
//        (
//            fieldName,
//            runTime.timeName(),
//            //runTime.mesh() 
//            // Needs runTime.findInstance()
//            mesh,
//            IOobject::MUST_READ
//        );
//    }
//
//    return objects;
//}

//template<class Type>
//HashTable
//<
//    GeometricField<Type, fvPatchField, volMesh>
//>
////void 
//ReadFields
//(
//    //const Time& runTime, // To read the startTimeDir from
//    //const fvMesh& mesh
//    IOobjectList& objects,
//	const fvMesh& mesh
//)
//{
//    word fieldClassName
//    (
//        GeometricField<Type, fvPatchField, volMesh>::typeName
//    );
//
//    //const fileNameList& fields = readDir(runTime.timeName());
//    IOobjectList fields = objects.lookupClass(fieldClassName);
//
//    //Info << "runTime.timeName() = " << runTime.timeName() << endl;
//    //Info << "fields.size() = " << fields.size();
//
//    HashTable
//    <
//        GeometricField<Type, fvPatchField, volMesh>
//    >                
//    tof;
//
//    forAll(fields, fieldI)
//    {
//        GeometricField<Type, fvPatchField, volMesh> field
//        (
//            fields[fieldI],
//            mesh
//        );
//
//        //ttof().insert
//        tof.insert
//        (
//            fieldName, field
//        );
//    }
//
//    //return ttof();
//    return tof;
//}
