"""Make tournament_id big integer

Revision ID: 8c13ffacf474
Revises: 8f3803c51ffb
Create Date: 2019-02-03 16:31:42.481983

"""
from alembic import op
import sqlalchemy as sa


# revision identifiers, used by Alembic.
revision = '8c13ffacf474'
down_revision = '8f3803c51ffb'
branch_labels = None
depends_on = None

def upgrade():
    op.alter_column('tournaments', 'tournament_id', type_=sa.BigInteger)

def downgrade():
    op.alter_column('tournaments', 'tournament_id', type_=sa.Integer)

