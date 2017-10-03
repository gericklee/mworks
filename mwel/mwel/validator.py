from __future__ import division, print_function, unicode_literals

from . import get_component_info
from .analyzer import Component


class Validator(object):

    # Only type signatures with one of the following roots are handled in
    # MWParserTransformation.xsl.  Component declarations with any other
    # signature root are just stripped out by the XSLT transformation, which
    # can lead to confusing errors.  Therefore, we insist that all component
    # declarations have a type signature that starts with one of these roots.
    _valid_signature_roots = (
        'action',
        'bias_monitor',
        'block',
        'calibrator',
        'experiment',
        'filter',
        'folder',
        'iochannel',
        'iodevice',
        'list',
        'list_replicator',
        'protocol',
        'range_replicator',
        'resource',
        'sound',
        'staircase',
        'stimulus',
        'stimulus_group',
        'task_system',
        'task_system_state',
        'transition',
        'trial',
        'variable',
        )

    def __init__(self, error_logger):
        self.error_logger = error_logger
        self._component_info = get_component_info()
        self._signature_to_name = dict((info['signature'], info['name'])
                                       for info in self._component_info.values()
                                       if not info.get('abstract', False))
        self._name_to_signature = dict((name, sig) for (sig, name) in
                                       self._signature_to_name.items())
        self._parents = []

    def validate(self, cmpts):
        experiment = None
        protocols = []
        others = []

        for c in cmpts:
            if c.name == 'experiment':
                if not experiment:
                    experiment = c
                else:
                    self.error_logger("Experiment cannot contain more than "
                                      "one 'experiment' component",
                                      lineno = c.lineno,
                                      colno = c.colno,
                                      filename = c.filename)
            elif c.name == 'protocol':
                protocols.append(c)
            else:
                others.append(c)

            self._validate(c)

        if not experiment:
            experiment = Component(name='experiment', tag='Experiment')

        if not (protocols or experiment.children):
            protocols.append(Component(name='protocol', tag='Empty Protocol'))
        experiment.children.extend(protocols)

        return others + [experiment]

    def _validate(self, c):
        signature = c.name + ('/' + c.type if c.type else '')
        if c.name not in self._valid_signature_roots:
            self.error_logger("'%s' is not a valid component type signature" %
                              signature,
                              lineno = c.lineno,
                              colno = c.colno,
                              filename = c.filename)

        name = self._signature_to_name.get(signature)
        if name:
            info = self._component_info[name]
            for parent in reversed(self._parents):
                if parent not in info['allowed_parent']:
                    parent_sig = self._name_to_signature.get(parent, parent)
                    self.error_logger("Component '%s' is not allowed inside "
                                      "component '%s'" %
                                      (signature, parent_sig),
                                      lineno = c.lineno,
                                      colno = c.colno,
                                      filename = c.filename)
                    break
                parent_info = self._component_info.get(parent, {})
                if not parent_info.get('transient', False):
                    break
            else:
                # Protocols aren't really allowed at the top level, but we
                # accept them there out of convenience, and move them in to
                # the experiment component before completing validation
                if not (signature == 'protocol' or info.get('toplevel', False)):
                    self.error_logger("Component '%s' is not allowed at "
                                      "the top level" % signature,
                                      lineno = c.lineno,
                                      colno = c.colno,
                                      filename = c.filename)
        else:
            # If we don't have any info for the given component, we can't
            # validate its parent
            pass

        self._parents.append(name or signature)
        try:
            for child in c.children:
                self._validate(child)
        finally:
            self._parents.pop()
