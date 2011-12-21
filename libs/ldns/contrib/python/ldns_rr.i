/******************************************************************************
 * ldns_rr.i: LDNS resource records (RR), RR list
 *
 * Copyright (c) 2009, Zdenek Vasicek (vasicek AT fit.vutbr.cz)
 *                     Karel Slany    (slany AT fit.vutbr.cz)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the organization nor the names of its
 *       contributors may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

%typemap(in,numinputs=0,noblock=1) (ldns_rr **)
{
 ldns_rr *$1_rr;
 $1 = &$1_rr;
}
          
/* result generation */
%typemap(argout,noblock=1) (ldns_rr **) 
{
  $result = SWIG_Python_AppendOutput($result, SWIG_NewPointerObj(SWIG_as_voidptr($1_rr), SWIGTYPE_p_ldns_struct_rr, SWIG_POINTER_OWN |  0 ));
}

%nodefaultctor ldns_struct_rr; //no default constructor & destructor
%nodefaultdtor ldns_struct_rr;

%ignore ldns_struct_rr::_rdata_fields;

%newobject ldns_rr_clone;
%newobject ldns_rr_pop_rdf;
%delobject ldns_rr_free;

%rename(ldns_rr) ldns_struct_rr;

#ifdef LDNS_DEBUG
%rename(__ldns_rr_free) ldns_rr_free;
%inline %{
void _ldns_rr_free (ldns_rr* r) {
   printf("******** LDNS_RR free 0x%lX ************\n", (long unsigned int)r);
   ldns_rr_free(r);
}
%}
#else
%rename(_ldns_rr_free) ldns_rr_free;
#endif

%newobject ldns_rr2str;
%newobject ldns_rr_type2str;
%newobject ldns_rr_class2str;
%newobject ldns_read_anchor_file;

%rename(_ldns_rr_new_frm_str) ldns_rr_new_frm_str;
%rename(_ldns_rr_new_frm_fp_l) ldns_rr_new_frm_fp_l;
%rename(_ldns_rr_new_frm_fp) ldns_rr_new_frm_fp;

%feature("docstring") ldns_struct_rr "Resource Record (RR)

The RR is the basic DNS element that contains actual data. This class allows to create RR and manipulate with the content."

%extend ldns_struct_rr {
 
 %pythoncode %{
        def __init__(self):
            raise Exception("This class can't be created directly. Please use: ldns_rr_new, ldns_rr_new_frm_type, new_frm_fp(), new_frm_fp_l(), new_frm_str() or new_question_frm_str")
       
        __swig_destroy__ = _ldns._ldns_rr_free

        #LDNS_RR_CONSTRUCTORS_#
        @staticmethod
        def new_frm_str(str, default_ttl=0, origin=None, prev=None, raiseException=True):
            """Creates an rr object from a string.

               The string should be a fully filled-in rr, like ownername [space] TTL [space] CLASS [space] TYPE [space] RDATA.
               
               :param str: the string to convert
               :param default_ttl: default ttl value for the rr. If 0 DEF_TTL will be used
               :param origin: when the owner is relative add this
               :param prev: the previous ownername
               :param raiseException: if True, an exception occurs in case a rr instance can't be created
               :returns: 
                  * rr - (ldnsrr) RR instance or None. If the object can't be created and raiseException is True, an exception occurs.
        
               **Usage**

               >>> import ldns
               >>> rr = ldns.ldns_rr.new_frm_str("www.nic.cz. IN A 192.168.1.1",300)
               >>> print rr
               www.nic.cz.  300  IN  A  192.168.1.1
            """
            status, rr, prev = _ldns.ldns_rr_new_frm_str_(str, default_ttl, origin, prev)
            if status != LDNS_STATUS_OK:
                if (raiseException): raise Exception("Can't create RR, error: %d" % status)
                return None
            return rr

        @staticmethod
        def new_question_frm_str(str, default_ttl=0, origin=None, prev=None, raiseException=True):
            """Creates an rr object from a string.

               The string is like new_frm_str but without rdata.

               :param str: the string to convert
               :param origin: when the owner is relative add this
               :param prev: the previous ownername
               :param raiseException: if True, an exception occurs in case a rr instance can't be created
               :returns: 
                  * rr - (ldnsrr) RR instance or None. If the object can't be created and raiseException is True, an exception occurs.
            """
            status, rr, prev = _ldns.ldns_rr_new_question_frm_str_(str, origin, prev)
            if status != LDNS_STATUS_OK:
                if (raiseException): raise Exception("Can't create RR, error: %d" % status)
                return None
            return rr

        @staticmethod
        def new_frm_str_prev(str, default_ttl=0, origin=None, prev=None, raiseException=True):
            """Creates an rr object from a string.

               The string should be a fully filled-in rr, like ownername [space] TTL [space] CLASS [space] TYPE [space] RDATA.
               
               :param str: the string to convert
               :param default_ttl: default ttl value for the rr. If 0 DEF_TTL will be used
               :param origin: when the owner is relative add this
               :param prev: the previous ownername
               :param raiseException: if True, an exception occurs in case a rr instance can't be created
               :returns: 
                  * rr - (ldnsrr) RR instance or None. If the object can't be created and raiseException is True, an exception occurs.
        
                  * prev - (ldns_rdf) ownername found in this string or None
            """
            status, rr, prev = _ldns.ldns_rr_new_frm_str_(str, default_ttl, origin, prev)
            if status != LDNS_STATUS_OK:
                if (raiseException): raise Exception("Can't create RR, error: %d" % status)
                return None
            return rr, prev

        @staticmethod
        def new_frm_fp(file, default_ttl=0, origin=None, prev=None, raiseException=True):
            """Creates a new rr from a file containing a string. 
              
               :param file: file pointer
               :param default_ttl: If 0 DEF_TTL will be used
               :param origin: when the owner is relative add this. 
               :param prev: when the owner is whitespaces use this.
               :param raiseException: if True, an exception occurs in case a resolver object can't be created
               :returns: 
                  * rr - (ldns_rr) RR object or None. If the object can't be created and raiseException is True, an exception occurs.

                  * ttl - (int) None or TTL if the file contains a $TTL directive 

                  * origin - (ldns_rdf) None or dname if the file contains a $ORIGIN directive

                  * prev - (ldns_rdf) None or updated value of prev parameter
            """
            res = _ldns.ldns_rr_new_frm_fp_l_(file, default_ttl, origin, prev, 0)
            if res[0] != LDNS_STATUS_OK:
                if (raiseException): raise Exception("Can't create RR, error: %d" % res[0])
                return None
            return res[1:]

        @staticmethod
        def new_frm_fp_l(file, default_ttl=0, origin=None, prev=None, raiseException=True):
            """Creates a new rr from a file containing a string. 
              
               :param file: file pointer
               :param default_ttl: If 0 DEF_TTL will be used
               :param origin: when the owner is relative add this. 
               :param prev: when the owner is whitespaces use this.
               :param raiseException: if True, an exception occurs in case a resolver object can't be created
               :returns: 
                  * rr - (ldns_rr) RR object or None. If the object can't be created and raiseException is True, an exception occurs.

                  * line - (int) line number (for debugging)

                  * ttl - (int) None or TTL if the file contains a $TTL directive 

                  * origin - (ldns_rdf) None or dname if the file contains a $ORIGIN directive

                  * prev - (ldns_rdf) None or updated value of prev parameter
            """
            res = _ldns.ldns_rr_new_frm_fp_l_(file, default_ttl, origin, prev, 1)
            if res[0] != LDNS_STATUS_OK:
                if (raiseException): raise Exception("Can't create RR, error: %d" % res[0])
                return None
            return res[1:]
        #_LDNS_RR_CONSTRUCTORS#


        def __str__(self):
            """converts the data in the resource record to presentation format"""
            return _ldns.ldns_rr2str(self)

        def __cmp__(self, other):
            """compares two rrs.
               
               The TTL is not looked at.
               
               :param other:
                   the second RR one
               :returns: (int) 0 if equal -1 if self comes before other RR +1 if other RR comes before self
            """
            return _ldns.ldns_rr_compare(self,other)

        def rdfs(self):
            """returns the list of rdata records."""
            for i in range(0,self.rd_count()):
                yield self.rdf(i)

        def get_function(self,rtype,pos):
            """return a specific rdf"""
            return _ldns.ldns_rr_function(rtype,self,pos)
            #parameters: ldns_rr_type,const ldns_rr *,size_t,
            #retvals: ldns_rdf *

        def set_function(self,rtype,rdf,pos):
            """set a specific rdf"""
            return _ldns.ldns_rr_set_function(rtype,self,rdf,pos)
            #parameters: ldns_rr_type,ldns_rr *,ldns_rdf *,size_t,
            #retvals: bool

        def print_to_file(self,output):
            """Prints the data in the resource record to the given file stream (in presentation format)."""
            _ldns.ldns_rr_print(output,self)
            #parameters: FILE *,const ldns_rr *,

        def get_type_str(self):
            """Converts an RR type value to its string representation, and returns that string."""
            return _ldns.ldns_rr_type2str(self.get_type())
            #parameters: const ldns_rr_type,

        def get_class_str(self):
            """Converts an RR class value to its string representation, and returns that string."""
            return _ldns.ldns_rr_class2str(self.get_class())
            #parameters: const ldns_rr_class,

        @staticmethod
        def dnskey_key_size_raw(keydata,len,alg):
            """get the length of the keydata in bits"""
            return _ldns.ldns_rr_dnskey_key_size_raw(keydata,len,alg)
            #parameters: const unsigned char *,const size_t,const ldns_algorithm,
            #retvals: size_t

        def write_to_buffer(self,buffer,section):
            """Copies the rr data to the buffer in wire format.
               
               :param buffer: buffer to append the result to buffer
               :param section:  the section in the packet this rr is supposed to be in (to determine whether to add rdata or not)
               :returns: (ldns_status) ldns_status
            """
            return _ldns.ldns_rr2buffer_wire(buffer,self,section)
            #parameters: ldns_buffer *,const ldns_rr *,int,
            #retvals: ldns_status

        def write_to_buffer_canonical(self,buffer,section):
            """Copies the rr data to the buffer in wire format, in canonical format according to RFC3597 (every dname in rdata fields of RR's mentioned in that RFC will be lowercased).
               
               :param buffer: buffer to append the result to buffer
               :param section:  the section in the packet this rr is supposed to be in (to determine whether to add rdata or not)
               :returns: (ldns_status) ldns_status
            """
            return _ldns.ldns_rr2buffer_wire_canonical(buffer,self,section)
            #parameters: ldns_buffer *,const ldns_rr *,int,
            #retvals: ldns_status

        def write_data_to_buffer(self,buffer):
            """Converts an rr's rdata to wireformat, while excluding the ownername and all the stuff before the rdata.
               
               This is needed in DNSSEC keytag calculation, the ds calcalution from the key and maybe elsewhere.
               
               :param buffer: buffer to append the result to
               :returns: (ldns_status) ldns_status
            """
            return _ldns.ldns_rr_rdata2buffer_wire(buffer,self)
            #parameters: ldns_buffer *,const ldns_rr *,
            #retvals: ldns_status

        def write_rrsig_to_buffer(self,buffer):
            """Converts a rrsig to wireformat BUT EXCLUDE the rrsig rdata 

               This is needed in DNSSEC verification.
               
               :param buffer: buffer to append the result to
               :returns: (ldns_status) ldns_status
            """
            return _ldns.ldns_rrsig2buffer_wire(buffer,self)
            #parameters: ldns_buffer *,const ldns_rr *,
            #retvals: ldns_status

            #LDNS_RR_METHODS_#
        def a_address(self):
            """returns the address of a LDNS_RR_TYPE_A rr
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the address or NULL on failure
            """
            return _ldns.ldns_rr_a_address(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def a_set_address(self,f):
            """sets the address of a LDNS_RR_TYPE_A rr
               
               :param f:
                   the address to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_a_set_address(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def clone(self):
            """clones a rr and all its data
               
               :returns: (ldns_rr \*) the new rr or NULL on failure
            """
            return _ldns.ldns_rr_clone(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rr *
	    
        def compare_ds(self,rr2):
            """returns true of the given rr's are equal.
               
               Also returns true if one record is a DS that represents the same DNSKEY record as the other record
               
               :param rr2:
                   the second rr
               :returns: (bool) true if equal otherwise false
            """
            return _ldns.ldns_rr_compare_ds(self,rr2)
            #parameters: const ldns_rr *,const ldns_rr *,
            #retvals: bool

        def compare_no_rdata(self,rr2):
            """compares two rrs, up to the rdata.
               
               :param rr2:
                   the second one
               :returns: (int) 0 if equal -1 if rr1 comes before rr2 +1 if rr2 comes before rr1
            """
            return _ldns.ldns_rr_compare_no_rdata(self,rr2)
            #parameters: const ldns_rr *,const ldns_rr *,
            #retvals: int

        def dnskey_algorithm(self):
            """returns the algorithm of a LDNS_RR_TYPE_DNSKEY rr
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the algorithm or NULL on failure
            """
            return _ldns.ldns_rr_dnskey_algorithm(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def dnskey_flags(self):
            """returns the flags of a LDNS_RR_TYPE_DNSKEY rr
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the flags or NULL on failure
            """
            return _ldns.ldns_rr_dnskey_flags(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def dnskey_key(self):
            """returns the key data of a LDNS_RR_TYPE_DNSKEY rr
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the key data or NULL on failure
            """
            return _ldns.ldns_rr_dnskey_key(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def dnskey_key_size(self):
            """get the length of the keydata in bits
               
               :returns: (size_t) the keysize in bits
            """
            return _ldns.ldns_rr_dnskey_key_size(self)
            #parameters: const ldns_rr *,
            #retvals: size_t

        def dnskey_protocol(self):
            """returns the protocol of a LDNS_RR_TYPE_DNSKEY rr
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the protocol or NULL on failure
            """
            return _ldns.ldns_rr_dnskey_protocol(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def dnskey_set_algorithm(self,f):
            """sets the algorithm of a LDNS_RR_TYPE_DNSKEY rr
               
               :param f:
                   the algorithm to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_dnskey_set_algorithm(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def dnskey_set_flags(self,f):
            """sets the flags of a LDNS_RR_TYPE_DNSKEY rr
               
               :param f:
                   the flags to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_dnskey_set_flags(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def dnskey_set_key(self,f):
            """sets the key data of a LDNS_RR_TYPE_DNSKEY rr
               
               :param f:
                   the key data to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_dnskey_set_key(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def dnskey_set_protocol(self,f):
            """sets the protocol of a LDNS_RR_TYPE_DNSKEY rr
               
               :param f:
                   the protocol to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_dnskey_set_protocol(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def get_class(self):
            """returns the class of the rr.
               
               :returns: (ldns_rr_class) the class of the rr
            """
            return _ldns.ldns_rr_get_class(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rr_class

        def get_type(self):
            """returns the type of the rr.
               
               :returns: (ldns_rr_type) the type of the rr
            """
            return _ldns.ldns_rr_get_type(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rr_type

        def label_count(self):
            """counts the number of labels of the ownername.
               
               :returns: (uint8_t) the number of labels
            """
            return _ldns.ldns_rr_label_count(self)
            #parameters: ldns_rr *,
            #retvals: uint8_t

        def mx_exchange(self):
            """returns the mx host of a LDNS_RR_TYPE_MX rr
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the name of the MX host or NULL on failure
            """
            return _ldns.ldns_rr_mx_exchange(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def mx_preference(self):
            """returns the mx pref.
               
               of a LDNS_RR_TYPE_MX rr
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the preference or NULL on failure
            """
            return _ldns.ldns_rr_mx_preference(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def ns_nsdname(self):
            """returns the name of a LDNS_RR_TYPE_NS rr
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the name or NULL on failure
            """
            return _ldns.ldns_rr_ns_nsdname(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def owner(self):
            """returns the owner name of an rr structure.
               
               :returns: (ldns_rdf \*) ldns_rdf *
            """
            return _ldns.ldns_rr_owner(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def pop_rdf(self):
            """removes a rd_field member, it will be popped from the last position.
               
               :returns: (ldns_rdf \*) rdf which was popped (null if nothing)
            """
            return _ldns.ldns_rr_pop_rdf(self)
            #parameters: ldns_rr *,
            #retvals: ldns_rdf *

        def push_rdf(self,f):
            """sets rd_field member, it will be placed in the next available spot.
               
               :param f:
               :returns: (bool) bool
            """
            return _ldns.ldns_rr_push_rdf(self,f)
            #parameters: ldns_rr *,const ldns_rdf *,
            #retvals: bool

        def rd_count(self):
            """returns the rd_count of an rr structure.
               
               :returns: (size_t) the rd count of the rr
            """
            return _ldns.ldns_rr_rd_count(self)
            #parameters: const ldns_rr *,
            #retvals: size_t

        def rdf(self,nr):
            """returns the rdata field member counter.
               
               :param nr:
                   the number of the rdf to return
               :returns: (ldns_rdf \*) ldns_rdf *
            """
            return _ldns.ldns_rr_rdf(self,nr)
            #parameters: const ldns_rr *,size_t,
            #retvals: ldns_rdf *

        def rrsig_algorithm(self):
            """returns the algorithm of a LDNS_RR_TYPE_RRSIG RR
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the algorithm or NULL on failure
            """
            return _ldns.ldns_rr_rrsig_algorithm(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def rrsig_expiration(self):
            """returns the expiration time of a LDNS_RR_TYPE_RRSIG RR
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the expiration time or NULL on failure
            """
            return _ldns.ldns_rr_rrsig_expiration(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def rrsig_inception(self):
            """returns the inception time of a LDNS_RR_TYPE_RRSIG RR
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the inception time or NULL on failure
            """
            return _ldns.ldns_rr_rrsig_inception(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def rrsig_keytag(self):
            """returns the keytag of a LDNS_RR_TYPE_RRSIG RR
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the keytag or NULL on failure
            """
            return _ldns.ldns_rr_rrsig_keytag(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def rrsig_labels(self):
            """returns the number of labels of a LDNS_RR_TYPE_RRSIG RR
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the number of labels or NULL on failure
            """
            return _ldns.ldns_rr_rrsig_labels(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def rrsig_origttl(self):
            """returns the original TTL of a LDNS_RR_TYPE_RRSIG RR
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the original TTL or NULL on failure
            """
            return _ldns.ldns_rr_rrsig_origttl(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def rrsig_set_algorithm(self,f):
            """sets the algorithm of a LDNS_RR_TYPE_RRSIG rr
               
               :param f:
                   the algorithm to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_rrsig_set_algorithm(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def rrsig_set_expiration(self,f):
            """sets the expireation date of a LDNS_RR_TYPE_RRSIG rr
               
               :param f:
                   the expireation date to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_rrsig_set_expiration(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def rrsig_set_inception(self,f):
            """sets the inception date of a LDNS_RR_TYPE_RRSIG rr
               
               :param f:
                   the inception date to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_rrsig_set_inception(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def rrsig_set_keytag(self,f):
            """sets the keytag of a LDNS_RR_TYPE_RRSIG rr
               
               :param f:
                   the keytag to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_rrsig_set_keytag(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def rrsig_set_labels(self,f):
            """sets the number of labels of a LDNS_RR_TYPE_RRSIG rr
               
               :param f:
                   the number of labels to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_rrsig_set_labels(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def rrsig_set_origttl(self,f):
            """sets the original TTL of a LDNS_RR_TYPE_RRSIG rr
               
               :param f:
                   the original TTL to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_rrsig_set_origttl(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def rrsig_set_sig(self,f):
            """sets the signature data of a LDNS_RR_TYPE_RRSIG rr
               
               :param f:
                   the signature data to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_rrsig_set_sig(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def rrsig_set_signame(self,f):
            """sets the signers name of a LDNS_RR_TYPE_RRSIG rr
               
               :param f:
                   the signers name to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_rrsig_set_signame(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def rrsig_set_typecovered(self,f):
            """sets the typecovered of a LDNS_RR_TYPE_RRSIG rr
               
               :param f:
                   the typecovered to set
               :returns: (bool) true on success, false otherwise
            """
            return _ldns.ldns_rr_rrsig_set_typecovered(self,f)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: bool

        def rrsig_sig(self):
            """returns the signature data of a LDNS_RR_TYPE_RRSIG RR
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the signature data or NULL on failure
            """
            return _ldns.ldns_rr_rrsig_sig(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def rrsig_signame(self):
            """returns the signers name of a LDNS_RR_TYPE_RRSIG RR
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the signers name or NULL on failure
            """
            return _ldns.ldns_rr_rrsig_signame(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def rrsig_typecovered(self):
            """returns the type covered of a LDNS_RR_TYPE_RRSIG rr
               
               :returns: (ldns_rdf \*) a ldns_rdf* with the type covered or NULL on failure
            """
            return _ldns.ldns_rr_rrsig_typecovered(self)
            #parameters: const ldns_rr *,
            #retvals: ldns_rdf *

        def set_class(self,rr_class):
            """sets the class in the rr.
               
               :param rr_class:
                   set to this class
            """
            _ldns.ldns_rr_set_class(self,rr_class)
            #parameters: ldns_rr *,ldns_rr_class,
            #retvals: 

        def set_owner(self,owner):
            """sets the owner in the rr structure.
               
               :param owner:
            """
            _ldns.ldns_rr_set_owner(self,owner)
            #parameters: ldns_rr *,ldns_rdf *,
            #retvals: 

        def set_rd_count(self,count):
            """sets the rd_count in the rr.
               
               :param count:
                   set to this count
            """
            _ldns.ldns_rr_set_rd_count(self,count)
            #parameters: ldns_rr *,size_t,
            #retvals: 

        def set_rdf(self,f,position):
            """sets a rdf member, it will be set on the position given.
               
               The old value is returned, like pop.
               
               :param f:
               :param position:
                   the position the set the rdf
               :returns: (ldns_rdf \*) the old value in the rr, NULL on failyre
            """
            return _ldns.ldns_rr_set_rdf(self,f,position)
            #parameters: ldns_rr *,const ldns_rdf *,size_t,
            #retvals: ldns_rdf *

        def set_ttl(self,ttl):
            """sets the ttl in the rr structure.
               
               :param ttl:
                   set to this ttl
            """
            _ldns.ldns_rr_set_ttl(self,ttl)
            #parameters: ldns_rr *,uint32_t,
            #retvals: 

        def set_type(self,rr_type):
            """sets the type in the rr.
               
               :param rr_type:
                   set to this type
            """
            _ldns.ldns_rr_set_type(self,rr_type)
            #parameters: ldns_rr *,ldns_rr_type,
            #retvals: 

        def ttl(self):
            """returns the ttl of an rr structure.
               
               :returns: (uint32_t) the ttl of the rr
            """
            return _ldns.ldns_rr_ttl(self)
            #parameters: const ldns_rr *,
            #retvals: uint32_t

        def uncompressed_size(self):
            """calculates the uncompressed size of an RR.
               
               :returns: (size_t) size of the rr
            """
            return _ldns.ldns_rr_uncompressed_size(self)
            #parameters: const ldns_rr *,
            #retvals: size_t

            #_LDNS_RR_METHODS#
 %}
}

%nodefaultctor ldns_struct_rr_list; //no default constructor & destructor
%nodefaultdtor ldns_struct_rr_list;

%ignore ldns_struct_rr_list::_rrs;

%newobject ldns_rr_list_clone;
%newobject ldns_rr_list_pop_rr;
%newobject ldns_rr_list_pop_rr_list;
%newobject ldns_rr_list_pop_rrset;
%delobject ldns_rr_list_deep_free;
%delobject ldns_rr_list_free;

%rename(ldns_rr_list) ldns_struct_rr_list;
#ifdef LDNS_DEBUG
%rename(__ldns_rr_list_deep_free) ldns_rr_list_deep_free;
%rename(__ldns_rr_list_free) ldns_rr_list_free;
%inline %{
void _ldns_rr_list_free(ldns_rr_list* r) {
   printf("******** LDNS_RR_LIST deep free 0x%lX ************\n", (long unsigned int)r);
   ldns_rr_list_deep_free(r);
}
%}
#else
%rename(_ldns_rr_list_deep_free) ldns_rr_list_deep_free;
%rename(_ldns_rr_list_free) ldns_rr_list_free;
#endif

%exception ldns_rr_list_push_rr(ldns_rr_list *rr_list, const ldns_rr *rr) %{ $action if (result) Py_INCREF(obj1); %}
%exception ldns_rr_list_push_rr_list(ldns_rr_list *rr_list, const ldns_rr_list *push_list) %{ $action if (result) Py_INCREF(obj1); %}

%newobject ldns_rr_list2str;




%feature("docstring") ldns_struct_rr_list "List of Resource Records.

This class contains a list of RR's (see :class:`ldns.ldns_rr`).
"

%extend ldns_struct_rr_list {
 
 %pythoncode %{
        def __init__(self):
            self.this = _ldns.ldns_rr_list_new()
            if not self.this:
                raise Exception("Can't create new RR_LIST")
       
        __swig_destroy__ = _ldns._ldns_rr_list_free

        #LDNS_RR_LIST_CONSTRUCTORS_#
        @staticmethod
        def new_frm_file(filename="/etc/hosts", raiseException=True):
            """Creates an RR List object from a file content

               Goes through a file and returns a rr_list containing all the defined hosts in there.

               :param filename: the filename to use
               :returns: RR List object or None. If the object can't be created and raiseException is True, an exception occurs.

               **Usage**
                 >>> alist = ldns.ldns_rr_list.new_frm_file()
                 >>> print alist
                 localhost.	3600	IN	A	127.0.0.1
                 ...

            """
            rr = _ldns.ldns_get_rr_list_hosts_frm_file(filename)
            if (not rr) and (raiseException): raise Exception("Can't create RR List, error: %d" % status)
            return rr
        #_LDNS_RR_LIST_CONSTRUCTORS#

        def __str__(self):
            """converts a list of resource records to presentation format"""
            return _ldns.ldns_rr_list2str(self)

        def print_to_file(self,output):
            """print a rr_list to output param[in] output the fd to print to param[in] list the rr_list to print"""
            _ldns.ldns_rr_list_print(output,self)


        def to_canonical(self):
            """converts each dname in each rr in a rr_list to its canonical form."""
            _ldns.ldns_rr_list2canonical(self)
            #parameters: ldns_rr_list *,
            #retvals: 

        def rrs(self):
            """returns the list of rr records."""
            for i in range(0,self.rr_count()):
                yield self.rr(i)

        def is_rrset(self):
            """checks if an rr_list is a rrset."""
            return _ldns.ldns_is_rrset(self)

        def __cmp__(self,rrl2):
            """compares two rr listss.
               
               :param rrl2:
                   the second one
               :returns: (int) 0 if equal -1 if this list comes before rrl2 +1 if rrl2 comes before this list
            """
            return _ldns.ldns_rr_list_compare(self,rrl2)

        def write_to_buffer(self, buffer):
            """Copies the rr_list data to the buffer in wire format.
               
               :param buffer: output buffer to append the result to
               :returns: (ldns_status) ldns_status
            """
            return _ldns.ldns_rr_list2buffer_wire(buffer,self)

           #LDNS_RR_LIST_METHODS_#
        def cat(self,right):
            """concatenates two ldns_rr_lists together.
               
               This modifies rr list (to extend it and add the pointers from right).
               
               :param right:
                   the rightside
               :returns: (bool) a left with right concatenated to it
            """
            return _ldns.ldns_rr_list_cat(self,right)
            #parameters: ldns_rr_list *,ldns_rr_list *,
            #retvals: bool

        def cat_clone(self,right):
            """concatenates two ldns_rr_lists together, but makes clones of the rr's (instead of pointer copying).
               
               :param right:
                   the rightside
               :returns: (ldns_rr_list \*) a new rr_list with leftside/rightside concatenated
            """
            return _ldns.ldns_rr_list_cat_clone(self,right)
            #parameters: ldns_rr_list *,ldns_rr_list *,
            #retvals: ldns_rr_list *

        def clone(self):
            """clones an rrlist.
               
               :returns: (ldns_rr_list \*) the cloned rr list
            """
            return _ldns.ldns_rr_list_clone(self)
            #parameters: const ldns_rr_list *,
            #retvals: ldns_rr_list *

        def contains_rr(self,rr):
            """returns true if the given rr is one of the rrs in the list, or if it is equal to one
               
               :param rr:
                   the rr to check
               :returns: (bool) true if rr_list contains rr, false otherwise
            """
            return _ldns.ldns_rr_list_contains_rr(self,rr)
            #parameters: const ldns_rr_list *,ldns_rr *,
            #retvals: bool

        def owner(self):
            """Returns the owner domain name rdf of the first element of the RR If there are no elements present, NULL is returned.
               
               :returns: (ldns_rdf \*) dname of the first element, or NULL if the list is empty
            """
            return _ldns.ldns_rr_list_owner(self)
            #parameters: const ldns_rr_list *,
            #retvals: ldns_rdf *

        def pop_rr(self):
            """pops the last rr from an rrlist.
               
               :returns: (ldns_rr \*) NULL if nothing to pop. Otherwise the popped RR
            """
            return _ldns.ldns_rr_list_pop_rr(self)
            #parameters: ldns_rr_list *,
            #retvals: ldns_rr *

        def pop_rr_list(self,size):
            """pops an rr_list of size s from an rrlist.
               
               :param size:
                   the number of rr's to pop
               :returns: (ldns_rr_list \*) NULL if nothing to pop. Otherwise the popped rr_list
            """
            return _ldns.ldns_rr_list_pop_rr_list(self,size)
            #parameters: ldns_rr_list *,size_t,
            #retvals: ldns_rr_list *

        def pop_rrset(self):
            """pops the first rrset from the list, the list must be sorted, so that all rr's from each rrset are next to each other
               
               :returns: (ldns_rr_list \*) 
            """
            return _ldns.ldns_rr_list_pop_rrset(self)
            #parameters: ldns_rr_list *,
            #retvals: ldns_rr_list *

        def push_rr(self,rr):
            """pushes an rr to an rrlist.
               
               :param rr:
                   the rr to push
               :returns: (bool) false on error, otherwise true
            """
            return _ldns.ldns_rr_list_push_rr(self,rr)
            #parameters: ldns_rr_list *,const ldns_rr *,
            #retvals: bool

        def push_rr_list(self,push_list):
            """pushes an rr_list to an rrlist.
               
               :param push_list:
                   the rr_list to push
               :returns: (bool) false on error, otherwise true
            """
            return _ldns.ldns_rr_list_push_rr_list(self,push_list)
            #parameters: ldns_rr_list *,const ldns_rr_list *,
            #retvals: bool

        def rr(self,nr):
            """returns a specific rr of an rrlist.
               
               :param nr:
                   return this rr
               :returns: (ldns_rr \*) the rr at position nr
            """
            return _ldns.ldns_rr_list_rr(self,nr)
            #parameters: const ldns_rr_list *,size_t,
            #retvals: ldns_rr *

        def rr_count(self):
            """returns the number of rr's in an rr_list.
               
               :returns: (size_t) the number of rr's
            """
            return _ldns.ldns_rr_list_rr_count(self)
            #parameters: const ldns_rr_list *,
            #retvals: size_t

        def set_rr(self,r,count):
            """set a rr on a specific index in a ldns_rr_list
               
               :param r:
                   the rr to set
               :param count:
                   index into the rr_list
               :returns: (ldns_rr \*) the old rr which was stored in the rr_list, or NULL is the index was too large set a specific rr
            """
            return _ldns.ldns_rr_list_set_rr(self,r,count)
            #parameters: ldns_rr_list *,const ldns_rr *,size_t,
            #retvals: ldns_rr *

        def set_rr_count(self,count):
            """sets the number of rr's in an rr_list.
               
               :param count:
                   the number of rr in this list
            """
            _ldns.ldns_rr_list_set_rr_count(self,count)
            #parameters: ldns_rr_list *,size_t,
            #retvals: 

        def sort(self):
            """sorts an rr_list (canonical wire format).
               
               the sorting is done inband.
            """
            _ldns.ldns_rr_list_sort(self)
            #parameters: ldns_rr_list *,
            #retvals: 

        def subtype_by_rdf(self,r,pos):
            """Return the rr_list which matches the rdf at position field.
               
               Think type-covered stuff for RRSIG
               
               :param r:
                   the rdf to use for the comparison
               :param pos:
                   at which position can we find the rdf
               :returns: (ldns_rr_list \*) a new rr list with only the RRs that match
            """
            return _ldns.ldns_rr_list_subtype_by_rdf(self,r,pos)
            #parameters: ldns_rr_list *,ldns_rdf *,size_t,
            #retvals: ldns_rr_list *

        def type(self):
            """Returns the type of the first element of the RR If there are no elements present, 0 is returned (LDNS_RR_TYPE_A).
               
               :returns: (ldns_rr_type) rr_type of the first element, or 0 if the list is empty
            """
            return _ldns.ldns_rr_list_type(self)
            #parameters: const ldns_rr_list *,
            #retvals: ldns_rr_type
            #_LDNS_RR_LIST_METHODS#
 %}
}

%newobject ldns_rr_descript;

%nodefaultctor ldns_struct_rr_descriptor; //no default constructor & destructor
%nodefaultdtor ldns_struct_rr_descriptor;
%rename(ldns_rr_descriptor) ldns_struct_rr_descriptor;




%feature("docstring") ldns_struct_rr_descriptor "Resource Record descriptor

This structure contains, for all rr types, the rdata fields that are defined."

%extend ldns_struct_rr_descriptor {
 %pythoncode %{
        def __init__(self):
            raise Exception("This class can't be created directly. Please use: ldns_rr_descript")
            #LDNS_RR_DESCRIPTOR_METHODS_#
        def field_type(self,field):
            """returns the rdf type for the given rdata field number of the rr type for the given descriptor.
               
               :param field:
                   the field number
               :returns: (ldns_rdf_type) the rdf type for the field
            """
            return _ldns.ldns_rr_descriptor_field_type(self,field)
            #parameters: const ldns_rr_descriptor *,size_t,
            #retvals: ldns_rdf_type

        def maximum(self):
            """returns the maximum number of rdata fields of the rr type this descriptor describes.
               
               :returns: (size_t) the maximum number of rdata fields
            """
            return _ldns.ldns_rr_descriptor_maximum(self)
            #parameters: const ldns_rr_descriptor *,
            #retvals: size_t

        def minimum(self):
            """returns the minimum number of rdata fields of the rr type this descriptor describes.
               
               :returns: (size_t) the minimum number of rdata fields
            """
            return _ldns.ldns_rr_descriptor_minimum(self)
            #parameters: const ldns_rr_descriptor *,
            #retvals: size_t

            #_LDNS_RR_DESCRIPTOR_METHODS#
 %}
}
 
