// Intel MKL VSL Random Number Generator
#include <mkl_vsl.h>

float MyRand_d ()
{
  // rand_r
  float randdd = (float) rand () / RAND_MAX;
  //float randdd = 0.002f;
  return randdd;
}


#define MYRAND (MyRand_d ())








void
MonteCarlo_d (ParaD pd, const int s1, const int s2max)
{

// not working, can't determine data size
//#pragma offload target(mic)

#pragma omp parallel for
  for (int r = pd.rep_begin; r <= pd.rep_end; ++r) {

    // constant
    // pointers
    ReplicaMC *rep = &pd.replica[r];
    const Ligand * const lig = &pd.lig[rep->idx_lig];
    const Protein * const prt = &pd.prt[rep->idx_prt];
    const Psp * const psp = pd.psp;
    const Kde * const kde = pd.kde;
    const Mcs * const mcs = pd.mcs;
    const EnePara * const enepara = pd.enepara;



    // temporary write-read
    float move_scale[6] MYALIGN; // translation x y z, rotation x y z
    float movematrix[6] MYALIGN; // translation x y z, rotation x y z
    float rot[3][3] MYALIGN; // rotz roty rotx
    float lig_x1[MAXLIG] MYALIGN;
    float lig_y1[MAXLIG] MYALIGN;
    float lig_z1[MAXLIG] MYALIGN;
    float lig_x2[MAXLIG] MYALIGN;
    float lig_y2[MAXLIG] MYALIGN;
    float lig_z2[MAXLIG] MYALIGN;


    // constant
    // ligand
    int lig_t[MAXLIG] MYALIGN;
    float lig_c[MAXLIG] MYALIGN;
    float lig_center[3] MYALIGN;


    // constant
    // protein
    float prt_pocket_center[3];


    // constant
    // mcs
    float mcs_tcc[MAX_MCS_ROW] MYALIGN;


    // constant
    // enepara
    float enepara_p1a[MAXTP1][MAXTP2] MYALIGN;
    float enepara_p2a[MAXTP1][MAXTP2] MYALIGN;
    float enepara_pmf0[MAXTP1][MAXTP2] MYALIGN;
    float enepara_pmf1[MAXTP1][MAXTP2] MYALIGN;
    float enepara_hdb0[MAXTP1][MAXTP2] MYALIGN;
    float enepara_hdb1[MAXTP1][MAXTP2] MYALIGN;
    float enepara_hpl0[MAXTP2] MYALIGN;
    float enepara_hpl1[MAXTP2] MYALIGN;
    float enepara_hpl2[MAXTP2] MYALIGN;
    float enepara_a_para[MAXWEI] MYALIGN;
    float enepara_b_para[MAXWEI] MYALIGN;
    float enepara_w[MAXWEI] MYALIGN;
    float enepara_lj0;
    float enepara_lj1;
    float enepara_el1;
    float enepara_el0;
    float enepara_a1;
    float enepara_b1;
    float enepara_kde2;
    float enepara_kde3_inv;


    // constant
    // scalars
    float sqrt_2_pi_inv;
    int lig_natom, prt_npoint, kde_npoint, mcs_nrow;


    // temporary write-read vars
    int is_accept_s;




    for (int bidx = 0; bidx < 6; ++bidx)
      move_scale[bidx] = pd.move_scale[bidx];

    for (int p = 0; p < MAXTP1; ++p) {
      for (int l = 0; l < MAXTP2; ++l) {
	enepara_p1a[p][l] = enepara->p1a[p][l];
	enepara_p2a[p][l] = enepara->p2a[p][l];
	enepara_pmf0[p][l] = enepara->pmf0[p][l];
	enepara_pmf1[p][l] = enepara->pmf1[p][l];
	enepara_hdb0[p][l] = enepara->hdb0[p][l];
	enepara_hdb1[p][l] = enepara->hdb1[p][l];
      }
    }

    for (int l = 0; l < MAXTP2; ++l) {
      enepara_hpl0[l] = enepara->hpl0[l];
      enepara_hpl1[l] = enepara->hpl1[l];
      enepara_hpl2[l] = enepara->hpl2[l];
    }
    
    for (int bidx = 0; bidx < MAXWEI - 1; ++bidx) {
      enepara_a_para[bidx] = enepara->a_para[bidx];
      enepara_b_para[bidx] = enepara->b_para[bidx];
      enepara_w[bidx] = enepara->w[bidx];
    }

    enepara_lj0 = enepara->lj0;
    enepara_lj1 = enepara->lj1;
    enepara_el1 = enepara->el1;
    enepara_el0 = enepara->el0;
    enepara_a1 = enepara->a1;
    enepara_b1 = enepara->b1;
    enepara_kde2 = enepara->kde2;
    enepara_kde3_inv = 1.0f / enepara->kde3;

    sqrt_2_pi_inv = -1.0f / sqrtf (2.0f * PI);
    lig_natom = lig->lig_natom;
    prt_npoint = prt->prt_npoint;
    kde_npoint = pd.kde_npoint;
    mcs_nrow = pd.mcs_nrow;
    is_accept_s = rep->is_accept;

    pd.record[r - pd.rep_begin].next_entry = 0; // reset the record's next_entry


    for (int l = 0; l < lig_natom; ++l) {
      lig_t[l] = lig->t[l];
      lig_c[l] = lig->c[l];
    }

    for (int bidx = 0; bidx < 3; ++bidx) {
      lig_center[bidx] = lig->center[bidx];
      prt_pocket_center[bidx] = prt->pocket_center[bidx];
    }

    for (int m = 0; m < mcs_nrow; ++m) {
      mcs_tcc[m] = mcs[m].tcc;
    }







    for (int s2 = 0; s2 < s2max; ++s2) {


      /////////////////////////////////////////////////////////////////////////////
      // record old states
      if (is_accept_s == 1) {
	rep->step = s1 + s2;

	const int rr = r - pd.rep_begin;
	const int next_entry = pd.record[rr].next_entry;
	pd.record[rr].replica[next_entry] = *rep;
	pd.record[rr].next_entry = next_entry + 1;
      }






      /////////////////////////////////////////////////////////////////////////////
      // move

      //#pragma unroll (6)
      for (int bidx = 0; bidx < 6; ++bidx) {
#if IS_AWAY == 0
	const float fixed_var = 0.0f;
#elif IS_AWAY == 1
	const float fixed_var = 44.5f;
#endif
	float moveamount = s2max != 1 ? MYRAND : fixed_var;
	movematrix[bidx] = move_scale[bidx] * moveamount + rep->movematrix[bidx];
      }

    
      // http://en.wikipedia.org/wiki/Euler_angles
      // http://upload.wikimedia.org/math/e/9/c/e9cf817bce9c1780216921cd93233459.png
      // http://upload.wikimedia.org/math/f/4/e/f4e55dc2c9581007648967d29b15121e.png
      const float sin1 = sinf (movematrix[3]);
      const float cos1 = cosf (movematrix[3]);
      const float sin2 = sinf (movematrix[4]);
      const float cos2 = cosf (movematrix[4]);
      const float sin3 = sinf (movematrix[5]);
      const float cos3 = cosf (movematrix[5]);
      rot[0][0] = cos1 * cos2;
      rot[0][1] = cos1 * sin2 * sin3 - cos3 * sin1;
      rot[0][2] = sin1 * sin3 + cos1 * cos3 * sin2;
      rot[1][0] = cos2 * sin1;
      rot[1][1] = cos1 * cos3 + sin1 * sin2 * sin3;
      rot[1][2] = cos3 * sin1 * sin2 - cos1 * sin3;
      rot[2][0] = -1 * sin2;
      rot[2][1] = cos2 * sin3;
      rot[2][2] = cos2 * cos3;


      // rotation, translation, coordinate system transformation
      for (int l = 0; l < lig_natom; ++l) {
	const float x1 = lig->x[l];
	const float y1 = lig->y[l];
	const float z1 = lig->z[l];
	lig_x1[l] = rot[0][0] * x1 + rot[0][1] * y1 + rot[0][2] * z1 + movematrix[0] + lig_center[0];
	lig_y1[l] = rot[1][0] * x1 + rot[1][1] * y1 + rot[1][2] * z1 + movematrix[1] + lig_center[1];
	lig_z1[l] = rot[2][0] * x1 + rot[2][1] * y1 + rot[2][2] * z1 + movematrix[2] + lig_center[2];

	const float x2 = lig->x2[l];
	const float y2 = lig->y2[l];
	const float z2 = lig->z2[l];
	lig_x2[l] = rot[0][0] * x2 + rot[0][1] * y2 + rot[0][2] * z2 + movematrix[0] + lig_center[0];
	lig_y2[l] = rot[1][0] * x2 + rot[1][1] * y2 + rot[1][2] * z2 + movematrix[1] + lig_center[1];
	lig_z2[l] = rot[2][0] * x2 + rot[2][1] * y2 + rot[2][2] * z2 + movematrix[2] + lig_center[2];
      }
  




      /////////////////////////////////////////////////////////////////////////////
      // calcenergy

      float evdw = 0.0f;
      float eele = 0.0f;
      float epmf = 0.0f;
      float epsp = 0.0f;
      float ehdb = 0.0f;
      float ehpc = 0.0f;


#if 1

      // lig loop, ~30
//#pragma unroll 8
      float ehpc1[lig_natom];
      for (int l = 0; l < lig_natom; ++l)
        ehpc1[l] = 0.0f;


      for (int l = 0; l < lig_natom; ++l) {
        const int lig__t = lig_t[l];

//#pragma unroll 4
        // prt loop, ~300
	for (int p = 0; p < prt_npoint; ++p) {
          const int prt__t = prt->t[p];

          const float dx = lig_x1[l] - prt->x[p];
          const float dy = lig_y1[l] - prt->y[p];
          const float dz = lig_z1[l] - prt->z[p];
          const float dst_pow2 = dx * dx + dy * dy + dz * dz;
          const float dst_pow4 = dst_pow2 * dst_pow2;
          const float dst = sqrtf (dst_pow2);


#if 1
          /* hydrophobic potential */

          if (prt->cdc[p] == 1 && dst_pow2 <= 81.0f) {
            ehpc1[l] += prt->hpp[p] *
              (1.0f - (3.5f / 81.0f * dst_pow2 -
                       4.5f / 81.0f / 81.0f * dst_pow4 +
                       2.5f / 81.0f / 81.0f / 81.0f * dst_pow4 * dst_pow2 -
                       0.5f / 81.0f / 81.0f / 81.0f / 81.0f * dst_pow4 * dst_pow4));
          }
#endif



      

#if 1
          /* L-J potential */
          // p1a[MAXTP2][MAXTP1]
          // p2a[MAXTP2][MAXTP1]
          {
	    const float p1 = enepara_p1a[prt__t][lig__t] / (dst_pow4 * dst_pow4 * dst);
	    const float p2 = enepara_p2a[prt__t][lig__t] / (dst_pow4 * dst_pow2);
	    const float p4 = p1 * enepara_lj0 * (1.0f + enepara_lj1 * dst_pow2) + 1.0f;
	    evdw += (p1 - p2) / p4;
          }
#endif




#if 1
          /* electrostatic potential */
          {
	    const float s1 = enepara_el1 * dst;
	    float g1;
	    if (s1 < 1)
	      g1 = enepara_el0 + enepara_a1 * s1 * s1 + enepara_b1 * s1 * s1 * s1;
	    else
	      g1 = 1.0f / s1;
	    eele += lig_c[l] * prt->ele[p] * g1;
          }
#endif


#if 1
          /* contact potential */
          // pmf0[MAXTP2][MAXTP1]
          // pmf1[MAXTP2][MAXTP1]
          // psp[MAXTP2][MAXPRO]

          const float dst_minus_pmf0 = dst - enepara_pmf0[prt__t][lig__t];

          epmf +=
            enepara_pmf1[prt__t][lig__t] /
            (1.0f + expf ((-0.5f * dst + 6.0f) * dst_minus_pmf0));


          /* pocket-specific potential */
          // the senmatics do not match with the original program:
          // if (found psp[][])
          //   accumulate to epsp
          // else
          //   do nothing
          if (prt->c[p] == 2 && dst_minus_pmf0 <= 0) {
            const int i1 = prt->seq3r[p];
            epsp += psp->psp[lig__t][i1]; // sparse matrix, indirect dereference

            // performance measuring
            // improve from 336 to 352, not worth doing
            //epsp += float (lig__t + i1);
          }
#endif


#if 1
          /* hydrogen bond potential */
          // hdb0[MAXTP2][MAXTP1]
          // hdb1[MAXTP2][MAXTP1]
          
          const float hdb0 = enepara_hdb0[prt__t][lig__t];
          if (hdb0 > 0.1f) {
            const float hdb1 = enepara_hdb1[prt__t][lig__t];
            const float hdb3 = (dst - hdb0) * hdb1;
            ehdb += hdb1 * expf (-0.5f * hdb3 * hdb3);
          }
#endif
        } // prt loop
      } // lig loop
#endif



      for (int l = 0; l < lig_natom; ++l) {
	/* hydrophobic restraits*/
	// hpl0[MAXTP2]
	// hpl1[MAXTP2]
	// hpl2[MAXTP2]
        const int lig__t = lig_t[l];
	const float hpc2 = (ehpc1[l] - enepara_hpl0[lig__t]) / enepara_hpl1[lig__t];
	ehpc += 0.5f * hpc2 * hpc2 - enepara_hpl2[lig__t];


        // create lig_hpl0[l]
        // create lig_hpl1[l]
        // create lig_hpl2[l]
      }


      float e[MAXWEI] MYALIGN;
      e[0] = evdw; // 0 - vdw 
      e[1] = eele; // 1 - ele
      e[2] = epmf; // 2 - pmf (CP)
      e[3] = epsp; // 3 - psp (PS CP)
      e[4] = ehdb * sqrt_2_pi_inv; // 4 - hdb (HB)
      e[5] = ehpc; // 5 - hpc (HP)








#if 1
      /* kde potential */
      // fully optimized

      float ekde = 0.0f;

      // lig loop, ~30
      for (int l = 0; l < lig_natom; ++l) {
	float ekde1 = 0.0f;
	int ekde2 = 0;
	const int lig__t = lig_t[l];

	// kde loop, ~400
	for (int k = 0; k < kde_npoint; ++k) {
	  if (lig__t == kde->t[k]) {
	    const float dx = lig_x1[l] - kde->x[k];
	    const float dy = lig_y1[l] - kde->y[k];
	    const float dz = lig_z1[l] - kde->z[k];
	    const float kde_dst_pow2 = dx * dx + dy * dy + dz * dz;
	    ekde1 += expf (enepara_kde2 * kde_dst_pow2);
	    ekde2++;
	  }
	} // kde loop

	if (ekde2 != 0)
	  ekde += (ekde1 / (float) ekde2);
        
      } // lig loop

      e[6] = ekde * enepara_kde3_inv;
#endif


#if 1
      /* position restraints */
      // fully optimized

      float elhm = 0.0f;

      // lhm loop, ~11
      //#pragma unroll 2
      for (int m = 0; m < mcs_nrow; ++m) {
	float elhm1 = 0.0f;
	int elhm2 = 0;

	for (int l = 0; l < lig_natom; ++l) {
          if (mcs[m].x[l + 1] != MCS_INVALID_COORD) {
            const float dx = lig_x2[l] - mcs[m].x[l + 1];
            const float dy = lig_y2[l] - mcs[m].y[l + 1];
            const float dz = lig_z2[l] - mcs[m].z[l + 1];
            elhm1 += dx * dx + dy * dy + dz * dz;
            elhm2++;
          }
        } // lig loop

        if (elhm2 != 0)
          elhm += mcs_tcc[m] * sqrtf (elhm1 / (float) elhm2);
      } // lhm loop

      e[7] = logf (elhm / mcs_nrow);
#endif


#if 1
      // fully optimized

      {
        float dst = 0;
        for (int bidx = 0; bidx < 3; ++bidx) {
          float d = lig_center[bidx] + movematrix[bidx] - prt_pocket_center[bidx];
          d = d * d;
          dst += d;
        }
	e[8] = sqrtf (dst);
      }
#endif



      // normalization
      for (int bidx = 0; bidx < 7; ++bidx)
        e[bidx] = e[bidx] / lig_natom;
      for (int bidx = 0; bidx < MAXWEI - 1; ++bidx)
	e[bidx] = enepara_a_para[bidx] * e[bidx] + enepara_b_para[bidx];
      float etotal = 0.0f;
      for (int bidx = 0; bidx < MAXWEI - 1; ++bidx)
	etotal += enepara->w[bidx] * e[bidx];
      e[MAXWEI - 1] = etotal;





      ////////////////////////////////////////////////////////////////////////
      // accept

      const float delta_energy = etotal - rep->energy[MAXWEI -1];
      const float beta = pd.temp[rep->idx_tmp].minus_beta;
      float rand = s2max != 1 ? MYRAND : 0.0f; // force to accept if s2max == 1
      is_accept_s = (rand < expf (delta_energy * beta));  // mybeta < 0

      if (is_accept_s == 1) {
        for (int bidx = 0; bidx < MAXWEI; ++bidx)
	  rep->energy[bidx] = e[bidx];
        for (int bidx = 0; bidx < 6; ++bidx)
          rep->movematrix[bidx] = movematrix[bidx];
      }


    } // s2 loop


    rep->is_accept = is_accept_s;

  } // replica loop

}

