"""add tournament info fields

Revision ID: dc5ee7723285
Revises: 185496e09438
Create Date: 2020-03-03 18:04:44.252824

"""
from alembic import op
import sqlalchemy as sa


# revision identifiers, used by Alembic.
revision = 'dc5ee7723285'
down_revision = '185496e09438'
branch_labels = None
depends_on = None


def upgrade():
    op.add_column('tournaments', sa.Column('name', sa.String, nullable=True))
    op.add_column('tournaments', sa.Column('location', sa.String, nullable=True))
    op.add_column('tournaments', sa.Column('date', sa.Date, nullable=True))

def downgrade():
    op.drop_column('tournaments', 'name')
    op.drop_column('tournaments', 'location')
    op.drop_column('tournaments', 'date')

