from . import ExporterModel
from . import LNA_model
from . import RuntimeModel


class LNA(ExporterModel.Module):
    def __init__(self, name=None):
        if not name:
            name = self.__class__.__name__
        super(LNA, self).__init__(name, visible=True)
        self.model = LNA_model

    def set_runtime_hooks(self):
        pass
