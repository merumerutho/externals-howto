/*
 * myphasor.c
 * @merumerutho (c) 2024 
 */


/**
 * include the interface to Pd
 */
#include "m_pd.h"
#include "math.h"

/**
 * define a new "class"
 */
static t_class *myphasor_tilde_class;


/**
 * this is the dataspace of our new object
 * the first element is the mandatory "t_object"
 * x_pan denotes the mixing-factor
 * "f" is a dummy and is used to be able to send floats AS signals.
 */
typedef struct _myphasor_tilde
{
  t_object x_obj;
  t_sample pitch;        // pitch factor
  t_float startloop;    // [0;1]
  t_float endloop;      // [0;1]
  t_float increase;     // 1/sampling_rate
  t_float prev_value;   // previous_value

  //t_inlet *p_startloop;
  //t_inlet *p_endloop;

  t_outlet *p_out;
} t_myphasor_tilde;


// Fractional part
static inline t_float fract(t_float x)
{
    return x - (int)x;
}

/**
 * this is the core of the object
 * this perform-routine is called for each signal block
 * the name of this function is arbitrary and is registered to Pd in the
 * myphasor_tilde_dsp() function, each time the DSP is turned on
 *
 * the argument to this function is just a pointer within an array
 * we have to know for ourselves how many elements inthis array are
 * reserved for us (hint: we declare the number of used elements in the
 * myphasor_tilde_dsp() at registration
 */
t_int *myphasor_tilde_perform(t_int *w)
{
  /* the first element is a pointer to the dataspace of this object */
  t_myphasor_tilde *x = (t_myphasor_tilde *)(w[1]);
  /* here comes the signalblock that will hold the output signal */
  t_sample    *out =      (t_sample *)(w[2]);
  /* all signalblocks are of the same length */
  int            n =             (int)(w[3]);
  /* just a counter */
  int i;



  // main routine
  for(i=0; i<n; i++)
    {
      out[i] = x->prev_value + (i*x->increase)*(x->pitch);
      out[i] = fract(out[i]);
    }
  x->prev_value += (n*x->increase)*(x->pitch);
  x->prev_value = fract(x->prev_value);

  /* return a pointer to the dataspace for the next dsp-object */
  return (w+4);
}


/**
 * register a special perform-routine at the dsp-engine
 * this function gets called whenever the DSP is turned ON
 * the name of this function is registered in myphasor_tilde_setup()
 */
void myphasor_tilde_dsp(t_myphasor_tilde *x, t_signal **sp)
{
  // update increase (1/sampling rate)
  x->increase = 1./sp[0]->s_sr;
  /* add myphasor_tilde_perform() to the DSP-tree;
     * the myphasor_tilde_perform() will expect "3" arguments (packed into an
     * t_int-array), which are:
     * the objects data-space,
     * 1 signal vectors (1 output signal)
     * and the length of the signal vectors (all vectors are of the same length)
     */
  dsp_add(myphasor_tilde_perform, 3, x,
          sp[0]->s_vec, sp[0]->s_n);
}

/**
 * this is the "destructor" of the class;
 * it allows us to free dynamically allocated ressources
 */
void myphasor_tilde_free(t_myphasor_tilde *x)
{
  /* free any ressources associated with the given outlet */
  outlet_free(x->p_out);
}

/**
 * this is the "constructor" of the class
 * the argument is the initial mixing-factor
 */
void *myphasor_tilde_new()
{
  t_myphasor_tilde *x = (t_myphasor_tilde *)pd_new(myphasor_tilde_class);

  x->pitch = 1.0;
  x->startloop = 0.0;
  x->endloop = 1.0;
  x->prev_value = 0.0;

  /* create a new signal-outlet */
  floatinlet_new(&x->x_obj, &x->pitch);
  x->p_out = outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

/**
 * define the function-space of the class
 * within a single-object external the name of this function is very special
 */
void myphasor_tilde_setup(void) {
    myphasor_tilde_class = class_new(gensym("myphasor~"),
        (t_newmethod)myphasor_tilde_new,
        (t_method)myphasor_tilde_free,
        sizeof(t_myphasor_tilde),
        CLASS_DEFAULT,
        A_DEFFLOAT, 0);

  /* whenever the audio-engine is turned on, the "myphasor_tilde_dsp()"
   * function will get called
   */
  class_addmethod(myphasor_tilde_class,
      (t_method)myphasor_tilde_dsp, gensym("dsp"), A_CANT, 0);
}
