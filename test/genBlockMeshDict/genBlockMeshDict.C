/**
 * Create blockMeshDict for upsetBillet
 *
 * z: minimum z
 * Z: max z in the inner block
 * ZZ: max z overall
 *
 * Same goes for r, R, and RR.
 * t and T refer to min and max in the peripheral direction (global Y dir).
 *
 * \author: Maalik (Ali Shayegh, ali@tensorfields.com), Richmond Hill, Dublin
 * \date: 22-11-2025
 */

#include "argList.H"
#include "foamTime.H"

#include "fvMesh.H"
#include "volMesh.H"
#include "surfaceMesh.H"

#include "GeometricField.H"
#include "vector.H"
#include "fvPatchField.H"
#include "fvsPatchField.H"

// Important
#include "surfaceFields.H"
// end Important

#include "fvc.H"
#include "fvm.H"
#include "fvMatrices.H"

#include "snGradScheme.H"

// Set precision
#include "IOmanip.H"

// On-demand
#include "mathematicalConstants.H"

// std libs
#include <vector>

using namespace Foam;

typedef std::vector<double> Vec;

std::vector<double>operator+
(
    const std::vector<double> x,
    const std::vector<double> y
)
{
    /// A Foam::vector rather than std::vector, to understand tmp behavior
    //tmp<Foam::vector> tv(new Foam::vector(0, 0, 0));
    std::vector<double> v {0, 0, 0};

    forAll(x, i)
    {
        //tv()[i] = x[i] + y[i];
        v[i] = x[i] + y[i];
    }

    //return tv();
    return v;
}

scalar deg2rad(const scalar& a)
{
    return (a / 180 * mathematicalConstant::pi);
}

/// Print header
void
printHeader(OFstream& ofs)
{
    ofs<< R"(
FoamFile
{
    class      dictionary;
    object     blockMeshDict;
    version    2.0;
    format     ascii;
})" <<"\n"<<"\n";
}

/// Print blocks
void
printBlock
(
    OFstream& ofs,
    const word& name,
    const HashTable<label>& points,
    const word& rtZ,
    const word& RtZ,
    const word& RTZ,
    const word& rtZ1,
    const word& rtZZ,
    const word& RtZZ,
    const word& RTZZ,
    const word& rtZZ1,
    label mr, label mt, label mz
)
{
    ofs <<"//"<<name<<"\n"
        << "hex "
        << "("
        << points[rtZ]<< " "<< points[RtZ]<< " "<< points[RTZ]
                      << " "<< points[rtZ1]<< " "
        << points[rtZZ]<< " "<< points[RtZZ]<< " "
        << points[RTZZ]<< " "<< points[rtZZ1];
    ofs << ") ("<<mr  <<" " << mt <<" " << mz << ") simpleGrading (1 1 1)\n";
}

void
printEdges
(
    OFstream& ofs,
    const label p1,
    const label p2,
    const std::vector<double>& v
)
{
    ofs <<"  "<<"  "<<"arc"
        <<"  "<< p1 <<" " << p2 <<" ("
        <<v[0]<<" " 
        <<v[1]<<" "
        <<v[2]<<")" <<'\n';
}

/// When you add a v=vector{x, y, z} to p=polar(r, t, phi),
/// - r-direction: Line connecting v to p has angle t with with y=0 plane
///   (negative angle when this line is in y<0 half-space)
/// - phi: Angle between v-p line and z=0 plane.
std::vector<double>
polar
(
    const scalar& r,
    const scalar& t,
    const scalar& phi
)
{
    return 
    std::vector<double>
    {
        r * Foam::cos(deg2rad(phi)) * Foam::cos(deg2rad(t)),
        r * Foam::cos(deg2rad(phi)) * Foam::sin(deg2rad(t)),
        r * Foam::sin(deg2rad(phi))
    };
}

void
printVertex
(
    OFstream& ofs,
    HashTable<label>& points,
    const Vec& cT,
    const word& name,
    label& count
)
{
    ofs <<"//"<<" " <<name<< endl;

    ofs <<"(" << cT[0] << "    " << cT[1] << "    " << cT[2]<< ")"<<" " <<"//"
        <<" " << ++count
        << endl;

    points.set(name, count);
}

int main(int argc, char* argv[])
{
    argList::validOptions.insert("convert","<scalar>");
    argList::validOptions.insert("nZT","<label>");
    argList::validOptions.insert("nRT","<label>");
    argList::validOptions.insert("nOL","<label>");
    argList::validOptions.insert("theta","<scalar>");
    argList::validOptions.insert("ZZ","<scalar>");
    argList::validOptions.insert("RR","<scalar>");
    argList::validOptions.insert("nPeriph","<label>");
    argList::validOptions.insert("nRd","<label>");
    argList::validOptions.insert("Rd","<scalar>");
    argList::validOptions.insert("dieThickness","<scalar>");

    argList args(argc, argv);

    Time runTime("controlDict", args.rootPath(), args.caseName());

    /// Input
        /// Number of points
        const label nP = 10;

        /// Number of total layers longitudinally
        scalar tmpConvert = 1;

        if (args.optionFound("convert"))
        {
            tmpConvert = (readScalar(IStringStream(args.options()["convert"])()));
        }

        const scalar convert = tmpConvert;

        /// Number of total layers longitudinally
        label tmpNZT = 12;

        if (args.optionFound("nZT"))
        {
            tmpNZT = label(readScalar(IStringStream(args.options()["nZT"])()));
        }

        const label nZT = tmpNZT;

        /// Number of total layers radially
        label tmpNRT = 12;

        if (args.optionFound("nRT"))
        {
            tmpNRT = label(readScalar(IStringStream(args.options()["nRT"])()));
        }

        const label nRT = tmpNRT;

        /// Number of upper-most layers and outer-most layers (they are equal)
        label tmpNOL = 1;

        if (args.optionFound("nOL"))
        {
            tmpNOL = label(readScalar(IStringStream(args.options()["nOL"])()));
        }

        const label nOL = tmpNOL;
        
        /// Aperture (degrees)
        scalar tmpT = 1;

        if (args.optionFound("theta"))
        {
            tmpT = label(readScalar(IStringStream(args.options()["theta"])()));
        }

        const scalar t = tmpT;

        /// Dimensions
        const scalar z  = 0;
        const scalar r  = 0;
        scalar tmpZZ = 15;

        if (args.optionFound("ZZ"))
        {
            tmpZZ = label(readScalar(IStringStream(args.options()["ZZ"])()));
        }

        const scalar ZZ = tmpZZ;

        scalar tmpRR = 15;

        if (args.optionFound("RR"))
        {
            tmpRR = label(readScalar(IStringStream(args.options()["RR"])()));
        }

        const scalar RR = tmpRR;

        /// N of cells in the curved direction in the corner
        label tmpNPeriph = 1;

        if (args.optionFound("nPeriph"))
        {
            tmpNPeriph = label(readScalar(IStringStream(args.options()["nPeriph"])()));
        }

        const scalar nPeriph = tmpNPeriph;

        /// Die properties

        scalar tmpDieTh = 1;

        if (args.optionFound("dieThickness"))
        {
            tmpNPeriph =
                (readScalar(IStringStream(args.options()["dieThickness"])()));
        }

        const scalar dieTh = tmpDieTh;

        scalar tmpNRd = 1;

        if (args.optionFound("nRd"))
        {
            tmpNRd =
                label
                (readScalar(IStringStream(args.options()["nRd"])()));
        }

        const scalar nRd = tmpNRd;

        scalar tmpRd = RR;

        if (args.optionFound("Rd"))
        {
            tmpRd =
                (readScalar(IStringStream(args.options()["Rd"])()));
        }

        const scalar Rd = tmpRd;

        /// Origin
        std::vector<double> rtz {r, 0, z};

    /// Calculate other dimensions

    /// Number of layers for the internal block
    const label nZ = nZT - nOL;
    const label nR = nRT - nOL;

    const scalar dz = ZZ / nZT;
    const scalar  Z = ZZ - nOL * dz; 

    const scalar drr = RR  / nRT;
    const scalar  dr = nOL * drr;
    const scalar   R = RR  - dr; 

    /// Die position
    const scalar Zd = ZZ + dieTh;

    /// Calculate points relative to the origin
    HashTable<label> points(nP);

    label count = -1;

    /// Order: min r to max r; min z to max z, min t to max t
    /// min r
        /// min z
        // rtz already difined as origin

        /// max z, mint
        std::vector<double> rtZ = rtz + std::vector<double>{0, 0, Z-z};

        /// max max z
        std::vector<double> rtZZ = rtz + std::vector<double>{0, 0, ZZ-z};

    /// max r
        /// min z
        std::vector<double> Rtz = rtz + polar(R, -t/2, 0);
        //+ std::vector<double>{R - r, -R * Foam::sin(deg2rad(t/2)), 0};
        std::vector<double> RTz = rtz + polar(R,  t/2, 0);
        //+ std::vector<double>{R - r,  R * Foam::sin(deg2rad(t/2)), 0};

        /// max z
        std::vector<double> RtZ = rtz + polar(R, -t/2, 0) + Vec{0, 0, Z};
        //+ std::vector<double>{R - r, -R * Foam::sin(deg2rad(t/2)), Z - z};
        std::vector<double> RTZ = rtz + polar(R,  t/2, 0) + Vec{0, 0, Z};
        //+ std::vector<double>{R - r,  R * Foam::sin(deg2rad(t/2)), Z - z};

        /// max max z
        std::vector<double> RtZZ = RtZ + Vec{0, 0, ZZ - Z};
        //rtz + std::vector<double>{R - r, -R * Foam::sin(deg2rad(t/2)), ZZ - z};
        std::vector<double> RTZZ = RTZ + Vec{0, 0, ZZ - Z};
        //rtz + std::vector<double>{R - r,  R * Foam::sin(deg2rad(t/2)), ZZ - z};

    /// max max r
        /// min z
        std::vector<double> RRtz = rtz + polar(RR, -t/2, 0);
        //+ std::vector<double>{RR - r, -RR * Foam::sin(deg2rad(t/2)), 0};
        std::vector<double> RRTz = rtz + polar(RR,  t/2, 0);
        //+ std::vector<double>{RR - r,  RR * Foam::sin(deg2rad(t/2)), 0};

        /// max z
        std::vector<double> RRtZ = rtz + polar(RR, -t/2, 0) + Vec{0, 0, Z};
        //rtz + std::vector<double>{RR - r, -RR * Foam::sin(deg2rad(t/2)), Z - z};
        std::vector<double> RRTZ = rtz + polar(RR,  t/2, 0) + Vec{0, 0, Z};
        //+ std::vector<double>{RR - r,  RR * Foam::sin(deg2rad(t/2)), Z - z};

        /// max max z: Does not exist; make them mid-curve points
        std::vector<double> ct = RtZ + polar(dr, -t/2, 45);
        std::vector<double> cT = RTZ + polar(dr,  t/2, 45);

        /// Die
        const Vec RdtZZ = rtZZ + polar(Rd, -t/2, 0);
        const Vec RdTZZ = rtZZ + polar(Rd,  t/2, 0);

        const Vec rtZd  = rtz  + Vec{0, 0, Zd};
        const Vec RdtZd = rtZd + polar(Rd, -t/2, 0);
        const Vec RdTZd = rtZd + polar(Rd,  t/2, 0);

        //    + std::vector<double>
        //      {
        //          dr * Foam::sin(deg2rad(45)),
        //         -(R + dr * Foam::sin(deg2rad(45))) * Foam::sin(deg2rad(t/2)),
        //          ZZ - z
        //      };

        //std::vector<double> cT = rtz
        //               + std::vector<double>{(R + dr * Foam::sin(deg2rad(45))) - r,
        //                  (R + dr * Foam::sin(deg2rad(45))) * Foam::sin(deg2rad(t/2)),
        //                  ZZ - z};

    /// Print to a file
    OFstream ofs("blockMeshDict");

    /// Print header
    printHeader(ofs);

    /// Print unit conversion
    ofs <<"convertToMeters "
        <<convert
        <<";" <<endl;

    /// Print vertices
    ofs<< "vertices\n"
        <<"(" <<"\n";

    /// r min
    ofs <<"(" << rtz[0] << "    " << rtz[1] << "    " << rtz[2]<< ")"<<" " <<"//"<<" " << ++count
        << endl;
    points.set("rtz", count);

    ofs <<"(" << rtZ[0] << "    " << rtZ[1] << "    " << rtZ[2]<< ")"<<" " <<"//"<<" " << ++count
        << endl;
    points.set("rtZ", count);

    ofs <<"(" << rtZZ[0] << "    " << rtZZ[1] << "    " << rtZZ[2]<< ")"<<" " <<"//"<<" " << ++count
        << endl;
    points.set("rtZZ", count);

    /// r max
        /// z, tmin
        ofs <<"(" << Rtz[0] << "    " << Rtz[1] << "    " << Rtz[2]<< ")"<<" " <<"//"<<" " << ++count
            << endl;
        points.set("Rtz", count);

        /// z, tmax
        ofs <<"(" << RTz[0] << "    " << RTz[1] << "    " << RTz[2]<< ")"<<" " <<"//"<<" " << ++count
            << endl;
        points.set("RTz", count);

        /// Z, t min
        ofs <<"(" << RtZ[0] << "    " << RtZ[1] << "    " << RtZ[2]<< ")"<<" " <<"//"<<" " << ++count
            << endl;
        points.set("RtZ", count);

        /// Z, t max
        ofs <<"(" << RTZ[0] << "    " << RTZ[1] << "    " << RTZ[2]<< ")"<<" " <<"//"<<" " << ++count
            << endl;
        points.set("RTZ", count);

        /// ZZ, t min
        ofs <<"(" << RtZZ[0] << "    " << RtZZ[1] << "    " << RtZZ[2]<< ")"<<" " <<"//"<<" " << ++count
            << endl;
        points.set("RtZZ", count);

        /// ZZ, t max
        ofs <<"(" << RTZZ[0] << "    " << RTZZ[1] << "    " << RTZZ[2]<< ")"<<" " <<"//"<<" " << ++count
            << endl;
        points.set("RTZZ", count);

    /// r max max

        /// zmin, t min
        ofs <<"(" << RRtz[0] << "    " << RRtz[1] << "    " << RRtz[2]<< ")"<<" " <<"//"<<" " << ++count
            << endl;
        points.set("RRtz", count);

        /// zmin, t max
        ofs <<"(" << RRTz[0] << "    " << RRTz[1] << "    " << RRTz[2]<< ")"<<" " <<"//"<<" " << ++count
            << endl;
        points.set("RRTz", count);

        /// z max, t min
        ofs <<"(" << RRtZ[0] << "    " << RRtZ[1] << "    " << RRtZ[2]<< ")"<<" " <<"//"<<" " << ++count
            << endl;
        points.set("RRtZ", count);

        /// z max, t max
        ofs <<"(" << RRTZ[0] << "    " << RRTZ[1] << "    " << RRTZ[2]<< ")"<<" " <<"//"<<" " << ++count
            << endl;
        points.set("RRTZ", count);

    /// mid-curve points
    ofs <<"(" << ct[0] << "    " << ct[1] << "    " << ct[2]<< ")"<<" " <<"//"<<" " << ++count
        << endl;
    points.set("ct", count);

    ofs <<"(" << cT[0] << "    " << cT[1] << "    " << cT[2]<< ")"<<" " <<"//"<<" " << ++count
        << endl;
    points.set("cT", count);

    /// Die
    printVertex(ofs, points, RdtZZ, "RdtZZ", count);
    printVertex(ofs, points, RdTZZ, "RdTZZ", count);
    printVertex(ofs, points, rtZd, "rtZd", count);
    printVertex(ofs, points, RdtZd, "RdtZd", count);
    printVertex(ofs, points, RdTZd, "RdTZd", count);

    ofs <<"\n"<<");"<<"\n";

    /// Print blocks
    ofs << 
    "blocks\n(\n";

    /// Inner block
    printBlock
    (
        ofs,
        "Die block",
        points,
        "rtZZ",
        "RdtZZ",
        "RdTZZ",
        "rtZZ",
        "rtZd",
        "RdtZd",
        "RdTZd",
        "rtZd",
        nRd, 1, 1
    );

    /// Inner block
    printBlock
    (
        ofs,
        "Innter block",
        points,
        "rtz",
        "Rtz",
        "RTz",
        "rtz",
        "rtZ",
        "RtZ",
        "RTZ",
        "rtZ",
        nR, 1, nZ
    );

    /// upper-most block
    printBlock
    (
        ofs,
        "Upper-most block",
        points,
        "rtZ",
        "RtZ",
        "RTZ",
        "rtZ",
        "rtZZ",
        "RtZZ",
        "RTZZ",
        "rtZZ",
        nR, 1, nOL
    );

    /// outer-most block
    printBlock
    (
        ofs,
        "Outer-most block",
        points,
        "Rtz",
        "RRtz",
        "RRTz",
        "RTz",
        "RtZ",
        "RRtZ",
        "RRTZ",
        "RTZ",
        nOL, 1, nZ
    );

    /// corner block
    printBlock
    (
        ofs,
        "Corner block",
        points,
        "RtZ",
        "RtZZ",
        "RRtZ",
        "RtZ",
        "RTZ",
        "RTZZ",
        "RRTZ",
        "RTZ",
        nOL, nPeriph, 1
    );

    ofs <<");"<<"\n";

    /// Print edges
    ofs << "edges\n"
        << "("<<"\n";

    printEdges(ofs, points["RtZZ"], points["RRtZ"], ct);
    printEdges(ofs, points["RTZZ"], points["RRTZ"], cT);

    ofs <<");"<<"\n";

    /// Print boundaries
    ofs << "patches\n"
        //<<"();"<<"\n";
        << R"(
(
    patch loading
    (
        (17 18 19 17)
    )

    patch billetContact
    (
        (8 2 7 8)
        (12 8 7 11)
        (10 12 11 9)
    )

    wedge billetFront
    (
        (0 1 6 4)
        (1 2 8 6)
        (6 8 12 6)
        (4 6 12 10)
    //)

    //wedge dieFront
    //(
        (2 17 19 16)
    )

    wedge billetBack
    (
        (3 5 1 0)
        (5 7 2 1)
        (5 11 7 5)
        (3 9 11 5)
    //)

    //wedge dieBack
    //(
        (2 15 18 17)
    )

    patch dieContact
    (
        (16 19 18 15)
        (2 16 15 2)
    )

	symmetryPlane symmPlane
	(
	    (10 9 3 4)
		(0 4 3 0)
	)

	empty axis
	(
	    (0 1 1 0)
		(1 2 2 1)
		(2 17 17 2)
	)
);

mergePatchPairs ();
        )""\n";

    return 0;
}
