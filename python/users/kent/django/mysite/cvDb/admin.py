# cv.django/admin.py was originally generated by the cvToSql program

""" 
This module tells the admin interface http://whereverItsMounted/admin/cvDb how to
wrap itself around our database models.  For the most part all it does is register each
model
"""

from django.contrib import admin
from models import *  # Forgive import *, but really it is the best thing in this case....

admin.site.register(CellLine)

class AbAdmin(admin.ModelAdmin):
"""Tell admin to not try to cram 300 targets into a selection gadget"""
    raw_id_fields = ["target"]

admin.site.register(Ab, AbAdmin)
admin.site.register(AbTarget)
admin.site.register(Antibody)
admin.site.register(MapAlgorithm)
admin.site.register(ReadType)
admin.site.register(InsertLength)
admin.site.register(FragSize)
admin.site.register(Localization)
admin.site.register(RnaExtract)
admin.site.register(Promoter)
admin.site.register(Control)
admin.site.register(Treatment)
admin.site.register(Protocol)
admin.site.register(Phase)
admin.site.register(Region)
admin.site.register(RestrictionEnzyme)
admin.site.register(View)
admin.site.register(DataType)
admin.site.register(Version)
admin.site.register(Strain)
admin.site.register(Age)
admin.site.register(Attic)
admin.site.register(Category)
admin.site.register(Sex)
admin.site.register(ObjStatus)
admin.site.register(Organism)
admin.site.register(TissueSourceType)
admin.site.register(SeqPlatform)
admin.site.register(Platform)
admin.site.register(Lab)
admin.site.register(Grantee)
admin.site.register(TypeOfTerm)
