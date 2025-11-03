            /// DEq
            //fvVectorMatrix DEq
            //(
            //    rho * fvm::d2dt2(D) // D = x - x0, so x replaced by D
            // == fvm::laplacian(mu, D)
            //  + fvc::div(sigma)
            //  - fvc::laplacian(mu, D)
            //  //+ fvm::laplacian(rho * (mesh.phi() - phi), U)
            //  + 1/runTime.deltaT() * (fvm::div(phi, D) - fvc::div(phi, D))
            //);
    
            ///
            /// vEq
            ///

            /// For now, the only spatially-implicit contribution comes from the
            /// convection term. 
            /// v = (D_new - D_old)/deltaT, for a single particle;
            /// This is the definition of v.
            /// If you are calculating
            /// (D_new - D_old)/deltaT at the same point of space, then
            /// (D_new - D_old)/deltaT = 0! (It is \partial_t x in Chadwick,
            /// p.52, eq. 12). 
            /// From
            /// Chadwich, p.52, eq. 12, the above formula is set in the Eulerian
            /// description for v, and alternatively can be written as
            /// v = x_dot, which in turn means that for Eulerian v,
            /// velocity at each points is the velocity of the particle
            /// occupying that point.
            /// Note that v in our formulation is not the material velocity,
            /// because we decided that \int_R \rho v dV is the momentum of the
            /// ARBITRARY region R.
            /// But actually v seems to be the material velocity!

            /// So, still not clear how to calculate D from v, and later, how to
            /// implicitely contribute sigma to the system.

            /// Ways to formulate vEq (Chapter: CALE, Eq. (6.15), Ali Shayegh,
            /// PhD Thesis):
            ///    fvm::ddt(rho * v)
            /// == div(sigma) + div((w-v), (rho*v)) + body_force
            ///
            /// For sigma, we have
            /// sigma_ij = mu (\partial_i u_j + \partial_j u_i) + lambda I_ij div(u)
            /// 
            /// so,
            /// 
            /// div(sigma) = div(mu, \partial_i u_j) + div(mu, \partial_j u_i)
            ///            + div(lambda, I_ij div(u))
            ///
            /// and substitute u in terms of v,
            ///
            /// div(sigma) = div(mu, \partial_i(u0_j + 0.5 deltaT (v_j+v0_j)))
            ///            + div(mu, \partial_j(u0_i + 0.5 deltaT (v_i+v0_i)))
            ///            + div(lambda, I_ij div(u))
            ///
            /// and in OpenFOAM syntax,
            ///
            /// div(sigma) = 0.5 * deltaT * fvm::laplacian(mu, v) 
            ///            + fvc::div(mu, fvc::grad(u))
            ///      // or + fvc::laplacian(mu, u)
            ///            + 0.5 * deltaT * fvc::div(mu, fvc::grad(v))
            ///      // or + 0.5 * deltaT * fvc::laplacian(mu, v)
            ///            + fvc::div(lambda, I * div(u))
            /// Continued in a paper.

            /// vEq initialized
            //fvMatrix<vector> vEq(); // If initialized, it seems impossible to
            /// change it

            //bool vEqAssembled = false;

            Info<< "Start assembling vEq ..."
                << endl;

            ///
            /// Option 1
            ///

            autoPtr<fvMatrix<vector> > vEqPtr
            (
                new fvMatrix<vector>
                (
                    //fvMatrix<vector> vEq
                    //(
                        rho * fvm::ddt(v) // Actually fvm::ddt(rho * v)
                     == 
                      //fvm::laplacian(mu, D) // consider ortho and non-ortho terms
                      //  runTime.deltaT() / 2 * fvm::laplacian(muS, v)
                      //+ runTime.deltaT() / 2 * fvc::laplacian(muS, v)

                      /// Unsteady Implementation
                        runTime.deltaT() / 2 * fvm::laplacian(convergenceFactor, v)
                      + runTime.deltaT() / 2 * fvc::laplacian(convergenceFactor, v)

                      /// Generalized form, for both steady and unsteady
                      //  pseudoDeltaT / 2 * fvm::laplacian(convergenceFactor, v)
                      //+ pseudoDeltaT / 2 * fvc::laplacian(convergenceFactor, v)

                      + fvc::div(sigma)
                      //+ convergenceFactor
                      //* (
                      //      fvc::laplacian(muS, U)
                      //    - muS * fvc::div(fvc::grad(U))
                      //  )
                      //- fvc::laplacian(muS, D)
                      /// So-called stabilization
                      //+ stabilizationFactor
                      //* (
                      //      runTime.deltaT() / 2 * fvc::laplacian(muS, v)
                      //    - runTime.deltaT() / 2 * muS * fvc::div(fvc::grad(v))
                      //  )
                      + fvm::div(phi, v) // Non-linearity
                    //);
                )
            );

            if (args.optionFound("formulaA"))
            {
                ///
                /// Option 2
                ///
    
                vEqPtr.reset
                (
                    new fvMatrix<vector>
                    (
                         fvm::ddt(rho, v) // Actually fvm::ddt(rho * v)
                      == 
                         /// This was the initial implementation to allow for av, av0
                         /// and au0 to be spatially variable if necessary, for now we
                         /// think of them fixed in space
                         //fvm::laplacian((a * muS + b * lambdaS), av * v)
                         fvm::laplacian((a * muS + b * lambdaS) * av, v)
                       + fvc::div(muS * au0 * fvc::grad(Dold)().T()())
                       + fvc::div(muS * av * fvc::grad(vPrevIter)().T()())
                       + fvc::div(muS * av0 * fvc::grad(vOld)().T()())
                       + fvc::grad(lambdaS * au0 * fvc::div(Dold))
                       + fvc::laplacian((a * muS + b * lambdaS) * av, vPrevIter)
                    // + fvc::div((a * muS + b * lambdaS), fvc::grad(av * v))
                       + fvc::laplacian(muS * av0, vOld)
                       + fvc::laplacian(muS * av, vPrevIter)
                       + fvc::laplacian(muS * au0, Dold)
                    // + fvc::div
                    //   (
                    //       muS,
                    //       fvc::grad(av0 * vOld + av * v + au0 * uOld)
                    //   )
                       + fvm::div(phi, v) // Non-linearity
                    )
                );
            }

            if (args.optionFound("formulaWave"))
            {
                vEqPtr.reset
                (
                    new fvMatrix<vector>
                    (
                        fvm::laplacian(mu, v) == rho * fvm::d2dt2(v)
                    )
                );
            }

            if (vEqPtr.empty())
            {
                FatalErrorIn("")
                    << "vEq not set."
                    << abort(FatalError);
            }

            fvMatrix<vector> vEq = vEqPtr();
