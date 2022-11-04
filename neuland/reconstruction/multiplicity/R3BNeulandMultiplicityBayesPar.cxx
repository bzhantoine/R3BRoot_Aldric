/******************************************************************************
 *   Copyright (C) 2019 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2019 Members of R3B Collaboration                          *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU General Public Licence (GPL) version 3,                *
 *                    copied verbatim in the file "LICENSE".                  *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#include "R3BNeulandMultiplicityBayesPar.h"
#include "FairLogger.h"
#include <numeric>
#include <string>

R3BNeulandMultiplicityBayesPar::R3BNeulandMultiplicityBayesPar(const char* name, const char* title, const char* context)
    : FairParGenericSet(name, title, context)
    , fHits({ 1000, 1000, 1000, 1000, 1000, 1000, 1000 })
    , fClusters({ 1000, 1000, 1000, 1000, 1000, 1000, 1000 })
    , fEdep({ 1000, 1000, 1000, 1000, 1000, 1000, 1000 })
    , fIsProperlyLoaded(false)
{
}

R3BNeulandMultiplicityBayesPar::~R3BNeulandMultiplicityBayesPar()
{
    // Note: Deleting stuff here or in clear() causes segfaults?
}

void R3BNeulandMultiplicityBayesPar::clear() {}

void R3BNeulandMultiplicityBayesPar::putParams(FairParamList* l)
{
    if (!l)
    {
        return;
    }

    for (int i = 0; i < NEULAND_MAX_MULT; i++)
    {
        l->add(TString::Format("NeulandMultiplicityBayesParHits-%d", i), fHits.at(i));
        l->add(TString::Format("NeulandMultiplicityBayesParClusters-%d", i), fClusters.at(i));
        l->add(TString::Format("NeulandMultiplicityBayesParEdep-%d", i), fEdep.at(i));
    }
}

Bool_t R3BNeulandMultiplicityBayesPar::getParams(FairParamList* l)
{
    if (!l)
    {
        return false;
    }

    for (int i = 0; i < NEULAND_MAX_MULT; i++)
    {
        l->fill(TString::Format("NeulandMultiplicityBayesParHits-%d", i), &fHits.at(i));
        l->fill(TString::Format("NeulandMultiplicityBayesParClusters-%d", i), &fClusters.at(i));
        l->fill(TString::Format("NeulandMultiplicityBayesParEdep-%d", i), &fEdep.at(i));
    }

    return true;
}

void R3BNeulandMultiplicityBayesPar::Fill(int n, int nHits, int nClusters, int Edep)
{
    fHits.at(n)[nHits]++;
    fClusters.at(n)[nClusters]++;
    fEdep.at(n)[Edep / 10]++;
}

R3BNeulandMultiplicity::MultiplicityProbabilities R3BNeulandMultiplicityBayesPar::GetProbabilities(int nHits,
                                                                                                   int nClusters,
                                                                                                   int Edep) const
{
    if (!fIsProperlyLoaded)
    {
        fIsProperlyLoaded = CheckIfProperlyLoaded();
    }

    R3BNeulandMultiplicity::MultiplicityProbabilities mult = { 0, 0, 0, 0, 0, 0, 0 };
    if (nHits == 0)
    {
        return mult;
    }

    double sum = 0;
    for (size_t i = 0; i < mult.size(); i++)
    {
        mult[i] =
            (double)fHits.at(i).At(nHits) * (double)fClusters.at(i).At(nClusters) * (double)fEdep.at(i).At(Edep / 10);
        sum += mult[i];
    }

    // Normalize so sum prob = 1
    for (double& i : mult)
    {
        i /= sum;
    }
    return mult;
}

bool R3BNeulandMultiplicityBayesPar::CheckIfProperlyLoaded() const
{
    if (std::accumulate(fHits.cbegin(), fHits.cend(), 0, [](int i, const TArrayI& a) { return i + a.GetSum(); }) < 1)
    {
        LOG(FATAL) << "R3BNeulandMultiplicityBayesPar: Empty dataset -> Not properly loaded!";
    }
    return true;
}

ClassImp(R3BNeulandMultiplicityBayesPar);
