"""Add save_time column

Revision ID: 185496e09438
Revises: 8c13ffacf474
Create Date: 2019-02-16 16:20:46.145948

"""
from alembic import op
import sqlalchemy as sa


# revision identifiers, used by Alembic.
revision = '185496e09438'
down_revision = '8c13ffacf474'
branch_labels = None
depends_on = None

def upgrade():
    op.add_column('tournaments', sa.Column('synced', sa.Boolean(), nullable=False, server_default="False"))
    op.add_column('tournaments', sa.Column('save_time', sa.DateTime(), nullable=True))

def downgrade():
    op.drop_column('tournaments', 'synced')
    op.drop_column('tournaments', 'save_time')
